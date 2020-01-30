#include "compiler.h"
#include "node.h"
#include <cstring>
#include <unistd.h> // chdir
#include <libgen.h> // dirname

// TODO functions and methods must be traversed once before compiling just so we have everything on record, for handling of casts and calls, etc.

// TODO call resolvetype inside the expressions's compile?!

int yyparse();
extern "C" FILE *yyin;

typedef CompileContext CC;

//void compile(Statement *stmts, CC* cc);
//Type *resolveType(Expression *exp, CC *cc);

CompileContext *compile(std::vector<Statement *> *stmts){
  CC *cc = new CC();
  
  cc->codes = stmts;

  /*
  for(auto s: *stmts){
    // precompile(s, cc);
  }
  */

  for(auto s: *stmts){
    s->compile(cc);
  }

  return cc;
}

std::vector<Statement *>* array_compile(std::vector<Statement *> *stmts, CC *cc){
  for(auto s: *stmts)
    s->compile(cc);

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

void ImportStatement::compile(CC *cc){
  // TODO improve

  ImportStatement *is = this; // TODO because I'm lazy
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

    strcat(newpath, "/");
    strcat(newpath, fname);

    // printf("No local file %s, trying %s\n", fname, path);

    yyin = fopen(newpath, "r");

    if(!yyin){
      printf("Could not import file \"%s\"\n", fname);
      printf("tried %s too\n", newpath);
      exit(1);
    }
  }
  chdir(dirname(fname)); // TODO change for windows
  extern std::vector<Statement *> *mainProgramNode;
  yyparse();
  auto p = mainProgramNode;

  p = array_compile(p, cc);
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

  // check for interfaces
  // resolve pointers.... augh
  Type *bt = baseType;
  int blevel = 0;
  int elevel = 0;
  Type *et = exprType;

  while(PointerType *pt = dynamic_cast<PointerType*>(bt)){
    bt = pt->base;
    blevel++;
  }

  while(PointerType *pt = dynamic_cast<PointerType*>(et)){
    et = pt->base;
    elevel++;
  }

  if(blevel + 1 == elevel)
  if(InterfaceType *it = dynamic_cast<InterfaceType*>(bt)){
    if(StructType *st = dynamic_cast<StructType*>(et)){
      // TODO change this later
      if(elevel != 1){
        printf("We don't support pointer of pointer operations between struct and interfaces, on line %d\n", n->lineno);
        exit(1);
      }
      
      // check for conversion manually
      // TODO
      InterfaceStatement *is = cc->getInterface(it->name);
      StructStatement *ss = cc->getStruct(st->name);
      if(!is){
        printf("Undefined interface %s on line %d\n", it->name->c_str(), n->lineno);
        exit(1);
      }
      if(!ss){
        printf("Undefined struct %s on line %d\n", st->name->c_str(), n->lineno);
        exit(1);
      }

      is->implements[*ss->name] = ss;

      for(auto m: *is->members){
        // check to see if struct has the same thing
        auto n = ss->methods[*m->name];
        if(!n){
          printf("Struct %s doesn't implement %s (%s) of interface %s on line %d\n",
                 st->displayName().c_str(), m->name->c_str(),
                 m->fType->displayName().c_str(),
                 it->displayName().c_str(), n->lineno);
          exit(1);
        }
        // TODO n without the first type
        FunctionType *nt = (FunctionType*)n->sign->fType->clone();
        nt->args->erase(nt->args->begin());
        if(m->fType->compatible(nt)!=OK){
          printf("Struct %s doesn't implement %s (%s) of interface %s on line %d\n",
                 st->displayName().c_str(), m->name->c_str(),
                 m->fType->displayName().c_str(),
                 it->displayName().c_str(), n->lineno);
          printf("%s implemented %s with type %s\n", st->displayName().c_str(),
                 m->name->c_str(), n->sign->fType->displayName().c_str());
          exit(1);
        }
        delete nt;
      }
      // If we reached here everything is fine, just return the expression
      // TODO we might want to pointer it later.
      return e;
    }
    // if we reached here, we can't do anything
  }

  // TODO check the other way, interface to struct

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

void FunctionDefine::compile(CC *cc){
  FunctionDefine *fd = this; // TODO because I'm lazy
  cc->blocks.push_back(new BlockContext());

  functionSignCompile(fd->sign, cc);

  cc->getBlock()->currentFunction = fd;

  for(auto s: *fd->body->stmts){
    s->compile(cc);
  }

  cc->blocks.pop_back();
}

void ReturnStatement::compile(CC *cc){
  // TODO handle void?

  auto rs = this; // TODO lazy

  if(!rs->exp)
    return;

  rs->exp->compile(cc);

  // return type
  Type *t = rs->exp->resolveType(cc);
  Type *ft = cc->getCurrentFunction()->sign->returnT;

  // check the return type using castcompile
  rs->exp = castCompile(t, ft, rs->exp, cc, rs, false);
}

void StructStatement::compile(CC *cc){
  auto ss = this; // TODO lazy
  cc->getBlock()->newStruct(ss->name, ss);
  // TODO do we want to do anything with the members?
  for(VariableDecl *s: *ss->members){
    s->compile(cc);
    if(s->exp)
      ss->has_initializer = true;
  }
}

void VariableDecl::compile(CC *cc){
  auto vd = this; // TODO lazy
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
          at->exp->compile(cc);
        }
      }
    }
  }
  // check exp type
  if(vd->exp){
    if(vd->t){
      vd->exp->compile(cc);
      auto etype = vd->exp->resolveType(cc);
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
      vd->exp->compile(cc);
      type = vd->exp->resolveType(cc);
    }
  } else {
    type = vd->t;
  }

  vd->t = type; // Just to ensure it is there, for use in codegen.

  // pointer level, how many pointers there are
  int level = 0;
  Type *tt = vd->t;
  while(PointerType *pt = dynamic_cast<PointerType*>(tt)){
    tt = pt->base;
    level++;
  }

  // handle structs and interfaces
  if(typeid(*tt).hash_code() == typeid(StructType).hash_code()){
    StructType *st = (StructType*)tt;
    if(StructStatement *ss = cc->getStruct(st->name)){
      // nothing to worry about, keep going.
    } else if(InterfaceStatement *is = cc->getInterface(st->name)){
      // replace with interface
      // TODO memory leak
      tt = new InterfaceType(is->name);
      while(level--)
        tt = new PointerType(tt);

      vd->t = tt;
    } else {
      // Struct or Interface not found
      printf("Unknown type %s on line %d\n", st->name->c_str(), vd->lineno);
      exit(1);
    }
  }

  // Add to block
  cc->getBlock()->newVar(vd->name, vd->t);
}

void FunctionSignature::compile(CC *cc){
    cc->blocks.push_back(new BlockContext());

    functionSignCompile(this, cc); // TODO super lazy

    BlockContext *bc = cc->blocks.back();
    cc->blocks.pop_back();
    delete bc;
    return;
}

void FunctionCallExpr::compile(CompileContext *cc){
  resolveType(cc);
  auto fce = this; // TODO lazy

  // check function exists
  auto fs = cc->getFunction(fce->name);
  if (!fs) {
    // it could be a function variable
    auto fv = cc->getVariableType(fce->name);

    if (!fv || typeid(*fv).hash_code() != typeid(FunctionType).hash_code()) {
      printf("Function not found %s\n", fce->name->c_str());
      exit(1);
    }
  }

  // generate arguments
  if (!fce->expr)
    fce->expr = new std::vector<Expression *>();

  for (auto e : *fce->expr)
    e->compile(cc);
}

void VariableAssign::compile(CC *cc){
  auto va = this;
  va->base->compile(cc);
  va->exp->compile(cc);

  // TODO move the cast to here.
  auto baseType = va->base->resolveType(cc);
  auto exprType = va->exp->resolveType(cc);
  va->exp = castCompile(exprType, baseType, va->exp, cc, va, false);
  // TODO
  return;
}

void IfStatement::compile(CompileContext *cc){
  auto is = this; // TODO lazy
  is->exp->compile(cc);
  is->i->compile(cc);
  if (is->e)
    is->e->compile(cc);
}

void CodeBlock::compile(CompileContext *cc){
  auto cb = this; // TODO lazy
  cc->blocks.push_back(new BlockContext());
  for (auto s : *cb->stmts) {
    s->compile(cc);
  }
  cc->blocks.pop_back();
}

void WhileStatement::compile(CC *cc){
  auto ws = this; // TODO lazy
  ws->exp->compile(cc);
  ws->w->compile(cc);
  return;
}

void IntValue::compile(CC *cc){
  resolveType(cc);
  // TODO check max size
  // TODO should we move atoi here?
}

void VariableExpr::compile(CC *cc){
  resolveType(cc);
  if (!cc->getVariableType(name)) {
    printf("Unknown variable %s in line %d\n", name->c_str(), lineno);
    exit(1);
  }
}

void BinaryOperation::compile(CC *cc){
  resolveType(cc);
  lhs->compile(cc);
  rhs->compile(cc);
  // TODO check to see if the op is valid on this type.
  // TODO
  // TODO I'd rather handle the op identification here, but it needs discussion
}

void StringValue::compile(CC *cc){
  resolveType(cc);
  // TODO
}

void MemberExpr::compile(CC *cc){
  resolveType(cc);
  e->compile(cc);
  // TODO check to see if mem is in the struct
  auto tmpe = e->resolveType(cc);

  int level = 0;

  while (PointerType *p = dynamic_cast<PointerType *>(tmpe)) {
    tmpe = p->base;
    level++;
  }
  this->level = level;
}

void ArrayExpr::compile(CC *cc){
  resolveType(cc);
  e->compile(cc);
  mem->compile(cc);
  // TODO check to see if ae->e is array
  // TODO check to see if ae->mem is number
}

void PointerAccessExpr::compile(CC *cc){
  resolveType(cc);
    exp->compile(cc);
}

void PointerExpr::compile(CC *cc){
  resolveType(cc);
  // TODO
}

void CastExpr::compile(CC *cc){
  resolveType(cc);
  // TODO use cast compile
  exp->compile(cc);
}

void DeferStatement::compile(CC *cc){
  // TODO Maybe we should handle defer here instead of codegen?
  s->compile(cc);
}

void SizeofExpr::compile(CC *cc){
  resolveType(cc);
  // TODO nothing?
}

void CompileStatement::compile(CompileContext *cc){
  s->compile(cc);

  // Now set it to be run
  cc->compiles.push_back(this);
}

void MemberStatement::compile(CC *cc){
  // Resolve the struct,
  // Add to it's methods
  MemberStatement *m = this; // TODO lazy
  StructStatement *s = cc->getStruct(m->name);
  if (!s) {
    printf("Struct %s not found on line %d\n", m->name->c_str(), m->lineno);
    exit(1);
  }
  std::string methodName(*m->f->sign->name);
  *m->f->sign->name = (*m->name) + "$" + (*m->f->sign->name);
  Type *selfT = new StructType(new std::string(m->name->c_str()));
  selfT = new PointerType(selfT);
  ArgDecl *self = new ArgDecl(new std::string("self"), selfT, false);
  m->f->sign->args->insert(m->f->sign->args->begin(), self);
  m->f->compile(cc);
  s->methods[methodName] = m->f;
}

void MethodCall::compile(CompileContext *cc){
  resolveType(cc);
  // it's basically a function call
  MethodCall *mc = this;

  Expression *e = mc->e;

  Type *tmpe = e->exprType;

  while (PointerType *p = dynamic_cast<PointerType *>(tmpe)) {
    tmpe = p->base;
    e = new PointerAccessExpr(e);
  }

  e->compile(cc);
  mc->expr->begin();
  mc->expr->insert(mc->expr->begin(), new PointerExpr(e));
  (*mc->expr)[0]->compile(cc);
  if (mc->f) {
    // This is a method call on structs
    mc->fce = new FunctionCallExpr(mc->f->sign->name, mc->expr);
  } else {
    // This is a method call on interfaces, we have to handle in codegen
  }
}

void InterfaceStatement::compile(CC *cc){
  // Register it like a struct
  auto is = this; // TODO lazy
  for (FunctionSignature *fs : *is->members) {
    fs->compile(cc);
  }

  cc->getBlock()->newInterface(is->name, is);
}

Type *IntValue::resolveType(CompileContext *cc){
  if(exprType)
    return exprType;
  exprType = new IntType();
  return exprType;
}

Type *StringValue::resolveType(CompileContext *cc){
  if(exprType)
    return exprType;
  exprType = new StringType();
  return exprType;
}

Type *VariableExpr::resolveType(CompileContext *cc){
  if(exprType)
    return exprType;
  Type *t = cc->getVariableType(name);
  if(!t){
      printf("Undefined variable %s at line %d\n", name->c_str(),
             lineno);
      exit(1);
  }
  exprType = t->clone();
  return exprType;
}

Type *BinaryOperation::resolveType(CompileContext *cc){
  if(exprType)
    return exprType;
  // TODO improve!
  exprType =lhs->resolveType(cc)->clone();
  return exprType;
}

Type *CastExpr::resolveType(CompileContext *cc){
  if(exprType)
    return exprType;
  exp->resolveType(cc); // TODO maybe even healthcheck?
  exprType = t->clone();
  return exprType;
}

Type *PointerExpr::resolveType(CompileContext *cc){
  if(exprType)
    return exprType;
  exprType = new PointerType(exp->resolveType(cc)->clone());
  return exprType;
}

Type *PointerAccessExpr::resolveType(CompileContext *cc){
  if(exprType)
    return exprType;
  PointerType *pt = (PointerType *)exp->resolveType(cc);
  exprType = pt->base->clone();
  return exprType;
}

Type *MemberExpr::resolveType(CompileContext *cc){
  if(exprType)
    return exprType;
  // TODO check to see if it's a pointer
  auto tmpe = this->e->resolveType(cc);

  while (PointerType *p = dynamic_cast<PointerType *>(tmpe)) {
    tmpe = p->base;
  }

  auto t = (StructType *)tmpe;
  auto ss = cc->getStruct(t->name);

  // error when ss not found
  if (!ss) {
    printf("Struct %s was not found, line %d\n", t->name->c_str(),
           this->lineno);
    exit(1);
  }

  for (auto m : *ss->members) {
    if (m->name->compare(*this->mem) == 0) {
      this->exprType = m->t->clone();
      return this->exprType;
    }
  }
  printf("Member type not found, memberExprType on line %d\n", lineno);
  exit(1);
  return nullptr;
}

Type *ArrayExpr::resolveType(CC *cc){
  if(exprType)
    return exprType;
  auto t = e->resolveType(cc);
  auto bt = (ArrayType*)t;
  exprType = bt->base->clone();
  return exprType;
}

Type *FunctionCallExpr::resolveType(CC *cc){
  if(exprType)
    return exprType;
  auto f = cc->getFunction(name);
  if(!f){
    // maybe it's a function variable
    auto f = cc->getVariableType(name);

    // verify it is of function type:
    if (f && typeid(*f).hash_code() == typeid(FunctionType).hash_code()) {
      // It's okay
      exprType = ((FunctionType *)f)->returnType->clone();
      return exprType;
    }

    printf("Unknown function %s at line %d\n", name->c_str(), lineno);
    exit(1);
  }
  exprType = f->returnT->clone();
  return exprType;
}

Type *SizeofExpr::resolveType(CompileContext *cc){
  if(exprType)
    return exprType;
  exprType = new IntType();
  return exprType;
}

Type *MethodCall::resolveType(CompileContext *cc){
  if(exprType)
    return exprType;
  MethodCall *mc = this; // TODO because I'm lazy
  e->compile(cc);

  auto tmpe = mc->e->resolveType(cc);

  while (PointerType *p = dynamic_cast<PointerType *>(tmpe)) {
    tmpe = p->base;
  }

  auto t = (StructType *)tmpe;

  auto st = cc->getStruct(t->name);

  // TODO it might be an interface

  if (!st) {
    auto it = cc->getInterface(t->name);
    if (!it) {
      // This will never happen
      printf("Unknown type %s on line %d\n", t->name, mc->lineno);
      printf("This should not happen\n");
      exit(1);
    }
    int ind = 0;
    for (auto f : *it->members) {
      if (f->name->compare(*mc->name) == 0) {
        mc->f = 0;
        mc->methodInd = ind;
        return f->returnT;
      }
      ind++;
    }

    printf("Interface %s doesn't have any method named %s, on line %d\n",
           t->name->c_str(), mc->name->c_str(), mc->lineno);
    exit(1);
  }

  FunctionDefine *f = st->methods[*mc->name];

  if (!f) {
    printf("Struct %s doesn't have any method named %s, on line %d\n",
           t->name->c_str(), mc->name->c_str(), mc->lineno);
    exit(1);
  }

  mc->f = f;

  return f->sign->returnT;
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
