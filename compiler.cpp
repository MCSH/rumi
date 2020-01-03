#include "compiler.h"
#include "node.h"
#include "type.h"
#include <cstring>
#include <unistd.h> // chdir
#include <libgen.h> // dirname


int yyparse();
extern "C" FILE *yyin;

typedef CompileContext CC;

void compile(Statement *stmts, CC* cc);
Type *resolveType(Expression *exp, CC *cc);

CompileContext *compile(std::vector<Statement *> *stmts){
  CC *cc = new CC();
  
  cc->codes = stmts;

  for(auto s: *stmts){
    compile(s, cc);
  }

  return cc;
}

std::vector<Statement *>* compile(std::vector<Statement *> *stmts, CC *cc){
  for(auto s: *stmts)
    compile(s, cc);

  return stmts;
}

CompileContext* compile(char *fileName){
  yyin = fopen(fileName, "r");

  chdir(dirname(fileName)); // TODO change for windows

  extern std::vector<Statement *> *mainProgramNode;
  yyparse();
  auto p = mainProgramNode;

  return compile(p);
}

void importCompile(ImportStatement *is, CC *cc){
  // TODO improve
  char *cwd = get_current_dir_name();
  int len = is->name->size()+1;
  int start = 0;
  const char *s = is->name->c_str();

  bool has_qt = false;
  bool hasExt = false;

  if(s[0] == '"'){
    has_qt = true;
    len-=2;
    start = 1;
  }

  if(s[len-1] == 'm' && s[len-2] == 'u' && s[len-3] == 'r' && s[len-4]=='.'){
    hasExt = true;
  } else {
    len += 4;
  }

  char *fname = (char*)malloc(sizeof(char*) *len);

  for(int i = start; i < len; i++){
    fname[i-start] = (*is->name)[i];
  }

  if(!hasExt){
    fname[len-2] = 'm';
    fname[len-3] = 'u';
    fname[len-4] = 'r';
    fname[len-5] = '.';
  }

  fname[len-1] = '\0';

  yyin = fopen(fname, "r");
  if(!yyin){
    // try the $RUMI_PATH
    char *path = getenv("RUMI_PATH");
    if(!path){
      printf("Could not import file \"%s\"\n", fname);
      printf("RUMI_PATH was not set either\n");
      exit(1);
    }

    char *newpath = (char*)malloc(strlen(path)+strlen(fname)+2);
    strcpy(newpath, path);
    // TODO memory leak
    // free(path);

    strcat(path, "/");
    strcat(path, fname);

    // printf("No local file %s, trying %s\n", fname, path);

    yyin = fopen(path, "r");

    if(!path){
      printf("Could not import file \"%s\"\n", fname);
      exit(1);
    }
  }
  chdir(dirname(fname)); // TODO change for windows
  extern std::vector<Statement *> *mainProgramNode;
  yyparse();
  auto p = mainProgramNode;

  p = compile(p, cc);
  // merge me with the mainProgram
  is->stmts = p;
  chdir(cwd); // Compile will change cwd, so go back for mod generating.
}

Expression *castCompile(Type *exprType, Type *baseType, Expression *e, CC *cc, Node *n, bool expl){
  // TODO, get a custom string for error reporting
  // Node n is wanted only for line no
  // Converting the third value, which is of the first type to second type.
  Compatibility c = baseType->compatible(exprType);

  if(c == ExpCast && !expl){
    printf("Can't implicity cast %s to %s\n",
           exprType->displayName().c_str(),
           baseType->displayName().c_str());
    printf("On line %d\n", n->lineno);
    exit(1);
  }

  if(c == UNCOMPATIBLE){
    printf("Uncompatible type conversion between %s and %s\n",
           exprType->displayName().c_str(),
           baseType->displayName().c_str());
    printf("On line %d\n", n->lineno);
    exit(1);
  }

  if(c == OK)
    return e;

  Expression* cast = new CastExpr(baseType, e);

  cast->exprType = baseType;

  return cast;
}

void functionSignCompile(FunctionSignature *fs, CC *cc){
  BlockContext *b = cc->getBlock();

  bool varargs = false;
  for(auto arg: *fs->args){
    if(varargs){
      // if this is true, a vardiac variable was defined in the middle, which is an error.
      printf("Vardiac variables should only be the last ones.\nError on line %d, function %s\n", fs->lineno, fs->name->c_str());
      exit(1);
    }
    if(!arg->vardiac)
      b->newVar(arg->name, arg->t);
    else {
      // TODO define the vardiac variable when we have arrays
      varargs = false;
    }
  }

  cc->getParentBlock()->newFunction(fs->name, fs);
}

void functionDefineCompile(FunctionDefine *fd, CC *cc){
  cc->blocks.push_back(new BlockContext());

  functionSignCompile(fd->sign, cc);

  cc->getBlock()->currentFunction = fd;

  for(auto s: *fd->body->stmts){
    compile(s, cc);
  }

  cc->blocks.pop_back();
}

void returnStmtCompile(ReturnStatement *rs, CC *cc){
  // TODO handle void?

  if(!rs->exp)
    return;

  compile(rs->exp, cc);

  // return type
  Type *t = resolveType(rs->exp, cc);
  Type *ft = cc->getCurrentFunction()->sign->returnT;

  // check the return type using castcompile
  rs->exp = castCompile(t, ft, rs->exp, cc, rs, false);
}

void structCompile(StructStatement *ss, CC *cc){
  cc->getBlock()->newStruct(ss->name, ss);
  // TODO do we want to do anything with the members?
  for(VariableDecl *s: *ss->members){
    compile(s, cc);
    if(s->exp)
      ss->has_initializer = true;
  }
}

void variableDeclCompile(VariableDecl *vd,CC *cc){
  Type *type;
  // check for arrays
  if (vd->t) {
    if (ArrayType *at = dynamic_cast<ArrayType *>(vd->t)){
      if(at->exp || at->count){
        if (auto *mt = dynamic_cast<IntValue *>(at->exp)) {
          at->count = atoi(mt->val->c_str());
          delete at->exp; // TODO problematic, remove in case of segfault
          at->exp = nullptr;
        } else {
          cc->getCurrentFunction()->dynamicStack = true;
          compile(at->exp, cc);
        }
      }
    }
  }
  // check exp type
  if(vd->exp){
    if(vd->t){
      compile(vd->exp, cc);
      auto etype = resolveType(vd->exp, cc);
      auto c = vd->t->compatible(etype);
      if(c==OK || c==ImpCast){
        delete etype; // TODO problematic, remove in case of segfault
      } else {
        printf("Uncompatible type covnersion from %s to %s at line %d\n",
               etype->displayName().c_str(), vd->t->displayName().c_str(), vd->lineno);
        exit(1);
      }
      type = vd->t;
    } else {
      compile(vd->exp, cc);
      type = resolveType(vd->exp, cc);
    }
  } else {
    type = vd->t;
  }

  vd->t = type; // Just to ensure it is there, for use in codegen.

  // Add to block
  cc->getBlock()->newVar(vd->name, type);
}

void compile(Statement *stmt, CC *cc){
  auto t = typeid(*stmt).hash_code();

  // TODO check resolve_type if it's an expression.
  // TODO this shouldn't be here if we call the resolveType of every expression ourselves.
  if(Expression *exp = dynamic_cast<Expression*>(stmt)){
    resolveType(exp, cc);
  }

  if(t == typeid(ImportStatement).hash_code())
    return importCompile((ImportStatement*) stmt, cc);

  if(t == typeid(FunctionDefine).hash_code())
    return functionDefineCompile((FunctionDefine*) stmt, cc);

  if(t == typeid(FunctionSignature).hash_code()){
    cc->blocks.push_back(new BlockContext());
    functionSignCompile((FunctionSignature *) stmt, cc);
    BlockContext *bc = cc->blocks.back();
    cc->blocks.pop_back();
    delete bc;
    return;
  }

  if(t == typeid(FunctionCallExpr).hash_code()){
    // TODO
    // TODO ensure type matches
    auto *fce= (FunctionCallExpr*) stmt;

    // check function exists
    auto fs = cc->getFunction(fce->name);
    if(!fs){
      // it could be a function variable
      auto fv = cc->getVariableType(fce->name);

      if(!fv || typeid(*fv).hash_code()!=typeid(FunctionType).hash_code()){
        printf("Function not found %s\n", fce->name->c_str());
        exit(1);
      }
    }

    // generate arguments
    if(!fce->expr)
      fce->expr = new std::vector<Expression *>();

    for(auto e: *fce->expr)
      compile(e, cc);
    return;
  }

  if(t == typeid(ReturnStatement).hash_code())
    return returnStmtCompile((ReturnStatement*) stmt, cc);

  if(t == typeid(VariableAssign).hash_code()){
    VariableAssign *va = (VariableAssign*) stmt;
    compile(va->base, cc);
    compile(va->exp, cc);

    // TODO move the cast to here.
    auto baseType = resolveType(va->base, cc);
    auto exprType = resolveType(va->exp, cc);
    va->exp = castCompile(exprType, baseType, va->exp, cc, va, false);
    // TODO
    return;
  }

  if(t == typeid(VariableDecl).hash_code()){
    return variableDeclCompile((VariableDecl*) stmt, cc);
  }

  if(t == typeid(IfStatement).hash_code()){
    // TODO
    // TODO check the expression type?
    IfStatement *is = (IfStatement *) stmt;
    compile(is->exp, cc);
    compile(is->i, cc);
    if(is->e)
      compile(is->e, cc);
    return;
  }

  if(t == typeid(CodeBlock).hash_code()){
    cc->blocks.push_back(new BlockContext());
    CodeBlock *cb = (CodeBlock *)stmt;
    for(auto s: *cb->stmts){
      compile(s, cc);
    }
    cc->blocks.pop_back();
    return;
  }

  if(t == typeid(WhileStatement).hash_code()){
    // TODO
    // TODO check the expression type?
    WhileStatement *ws = (WhileStatement*) stmt;
    compile(ws->exp, cc);
    compile(ws->w, cc);
    return;
  }

  if(t == typeid(StructStatement).hash_code()){
    structCompile((StructStatement*)stmt, cc);
    // TODO
    return;
  }

  if(t == typeid(IntValue).hash_code()){
    // TODO check max size
    // TODO should we move atoi here?
    return;
  }

  if(t == typeid(VariableExpr).hash_code()){
    auto ve = (VariableExpr*) stmt;
    // check to see if var is defined.
    if(!cc->getVariableType(ve->name)){
      printf("Unknown variable %s in line %d\n", ve->name->c_str(), ve->lineno);
      exit(1);
    }
    return;
  }

  if(t == typeid(BinaryOperation).hash_code()){
    auto bo = (BinaryOperation *) stmt;
    compile(bo->lhs, cc);
    compile(bo->rhs, cc);
    // TODO check to see if the op is valid on this type.
    // TODO
    // TODO I'd rather handle the op identification here, but it needs discussion
    return;
  }

  if(t == typeid(StringValue).hash_code()){
    // TODO
    return;
  }

  if(t == typeid(MemberExpr).hash_code()){
    MemberExpr *me = (MemberExpr*) stmt;
    compile(me->e, cc);
    compile(me->e, cc); // TODO repetition
    // TODO check to see if mem is in the struct
    return;
  }

  if(t == typeid(ArrayExpr).hash_code()){
    ArrayExpr *ae = (ArrayExpr*) stmt;
    compile(ae->e, cc);
    compile(ae->mem, cc);
    // TODO check to see if ae->e is array
    // TODO check to see if ae->mem is number
    return;
  }

  if(t == typeid(PointerAccessExpr).hash_code()){
    // TODO
    PointerAccessExpr *pae = (PointerAccessExpr*)stmt;
    compile(pae->exp, cc);
    return;
  }

  if(t == typeid(PointerExpr).hash_code()){
    // TODO
    return;
  }

  if(t == typeid(CastExpr).hash_code()){
    // TODO
    // TODO use cast compile
    CastExpr *ce = (CastExpr*) stmt;
    compile(ce->exp, cc);
    return;
  }

  if(t == typeid(DeferStatement).hash_code()){
    // TODO Maybe we should handle defer here instead of codegen?
    DeferStatement *ds = (DeferStatement*) stmt;
    compile(ds->s, cc);
    return;
  }

  if(t == typeid(SizeofExpr).hash_code()){
    // TODO no compilation?
    return;
  }

  if(t == typeid(CompileStatement).hash_code()){
    // First, compile the top_level
    CompileStatement *s = (CompileStatement*)stmt;
    compile(s->s, cc);

    // Now set it to be run
    cc->compiles.push_back(s);
    return;
  }

  if(t == typeid(MemberStatement).hash_code()){
    // Resolve the struct,
    // Add to it's methods
    MemberStatement *m = (MemberStatement*)stmt;
    StructStatement *s = cc->getStruct(m->name);
    if(!s){
      printf("Struct %s not found on line %d\n", m->name->c_str(), m->lineno);
      exit(1);
    }
    std::string methodName(*m->f->sign->name);
    *m->f->sign->name = (*m->name)+"$"+(*m->f->sign->name);
    Type *selfT = new StructType(new std::string(m->name->c_str()));
    selfT = new PointerType(selfT);
    ArgDecl *self = new ArgDecl(new std::string("self"), selfT, false);
    m->f->sign->args->insert(m->f->sign->args->begin(), self);
    compile(m->f, cc);
    s->methods[methodName] = m->f;
    return;
  }

  if(t == typeid(MethodCall).hash_code()){
    // it's basically a function call
    MethodCall *mc = (MethodCall*) stmt;


    Expression *e = mc->e;

    Type *tmpe = e->exprType;

    while(PointerType *p = dynamic_cast<PointerType*>(tmpe)){
      tmpe = p->base;
      e = new PointerAccessExpr(e);
    }

    compile(e, cc);
    mc->expr->begin();
    mc->expr->insert(mc->expr->begin(), new PointerExpr(e));
    compile((*mc->expr)[0], cc);
    mc->fce = new FunctionCallExpr(mc->f->sign->name, mc->expr);
    return;
  }

  printf("Undefined compile for statement %s at compiler::compile, lineno: %d\n", typeid(*stmt).name(), stmt->lineno);
  exit(1);
  
}

Type *resolveType(Expression *expr, CC *cc){
  // Don't compute it twice.
  if(expr->exprType)
    return expr->exprType;

  auto t = typeid(*expr).hash_code();

  if(t == typeid(IntValue).hash_code()){
    expr->exprType = new IntType();
    return expr->exprType;
  }

  if(t == typeid(StringValue).hash_code()){
    expr->exprType = new StringType();
    return expr->exprType;
  }

  if(t == typeid(VariableExpr).hash_code()){
    VariableExpr * ve= (VariableExpr*) expr;
    Type *t = cc->getVariableType(ve->name);
    if (!t) {
      printf("Undefined variable %s at line %d\n", ve->name->c_str(),
             ve->lineno);
      exit(1);
    }
    expr->exprType = t->clone();
    return expr->exprType;
  }

  if(t == typeid(BinaryOperation).hash_code()){
    BinaryOperation *bo = (BinaryOperation*) expr;
    // TODO complete this
    // 1 + 2.0 -> float
    expr->exprType = resolveType(bo->lhs, cc)->clone();
    return expr->exprType;
  }

  if(t == typeid(CastExpr).hash_code()){
    auto ce = (CastExpr*) expr;
    resolveType(ce->exp, cc); // TODO maybe even healthcheck?
    expr->exprType = ce->t->clone();
    return expr->exprType;
  }

  if(t == typeid(PointerExpr).hash_code()){
    PointerExpr *pe = (PointerExpr*)expr;
    expr->exprType = new PointerType(resolveType(pe->exp, cc)->clone());
    return expr->exprType;
  }

  if(t == typeid(PointerAccessExpr).hash_code()){
    PointerType *pt = (PointerType *) resolveType(((PointerAccessExpr*)expr)->exp, cc);
    expr->exprType = pt->base->clone();
    return expr->exprType;
  }

  if(t == typeid(MemberExpr).hash_code()){
    auto *me = (MemberExpr*) expr;
    // TODO check to see if it's a pointer
    auto tmpe = resolveType(me->e, cc);

    while(PointerType*p = dynamic_cast<PointerType*>(tmpe)){
      tmpe = p->base;
    }
    
    auto t = (StructType*) tmpe;
    auto ss = cc->getStruct(t->name);

    // error when ss not found
    if(!ss){
      printf("Struct %s was not found, line %d\n", t->name->c_str(), expr->lineno);
      exit(1);
    }

    for(auto m: *ss->members){
      if(m->name->compare(*me->mem) == 0){
        expr->exprType = m->t->clone();
        return expr->exprType;
      }
    }
    printf("Member type not found, memberExprType on line %d\n", me->lineno);
    exit(1);
  }

  if(t == typeid(ArrayExpr).hash_code()){
    auto *ae = (ArrayExpr*) expr;
    auto t = resolveType(ae->e, cc);
    // TODO check to see if it is an array
    auto bt = (ArrayType*) t;
    expr->exprType = bt->base->clone();
    return expr->exprType;
  }

  if(t == typeid(FunctionCallExpr).hash_code()){
    FunctionCallExpr *fce = (FunctionCallExpr *) expr;
    // TODO check for clone
    auto f = cc->getFunction(fce->name);
    if(!f){
      // maybe it's a function variable
      auto f = cc->getVariableType(fce->name);

      // verify it is of function type:
      if(f && typeid(*f).hash_code() == typeid(FunctionType).hash_code()){
        // It's okay
        expr->exprType = ((FunctionType*)f)->returnType->clone();
        return expr->exprType;
      }
      
      printf("Unknown function %s at line %d\n",
             fce->name->c_str(), fce->lineno);
      exit(1);
    }
    expr->exprType = f->returnT->clone();
    return expr->exprType;
  }

  if(t == typeid(SizeofExpr).hash_code()){
    return new IntType(); // TODO improve?
  }

  if(t == typeid(MethodCall).hash_code()){
    MethodCall *mc = (MethodCall*) expr;
    compile(mc->e, cc);

    auto tmpe  = resolveType(mc->e, cc);

    while(PointerType *p = dynamic_cast<PointerType*>(tmpe)){
      tmpe = p->base;
    }

    auto t = (StructType*) tmpe; 

    auto st = cc->getStruct(t->name);

    FunctionDefine *f = st->methods[*mc->name];

    if(!f){
      printf("Struct %s doesn't have any method named %s\n", t->name->c_str(), mc->name->c_str());
      exit(1);
    }

    mc->f = f;

    return f->sign->returnT;
  }

  printf("Undefined resolveType for expression %s at compiler::resolveType\n", typeid(*expr).name());
  exit(1);
}

Type *FunctionSignature::getType(){
  // TODO maybe cache this?
  if(fType)
    return fType;

  std::vector<Type *> *a=new std::vector<Type*>();

  for(auto arg: *args){
    a->push_back(arg->t->clone());
  }

  fType = new FunctionType(a, returnT->clone());
  return fType;
}
