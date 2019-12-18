#include "compiler.h"
#include "node.h"
#include "type.h"
#include <map>


int yyparse();
extern "C" FILE *yyin;

class BC { // Block Context
public:
  Type *returnType;
  std::map<std::string, Type *> vars;
  std::map<std::string, StructStatement *> structs;
  std::map<std::string, FunctionSignature *> functions;

  void newVar(std::string *name, Type *type){
    // TODO
    vars[*name] = type;
  }

  void newStruct(std::string *name, StructStatement *ss){
    // TODO
    structs[*name] = ss;
  }

  void newFunction(std::string *name, FunctionSignature *fs){
    // TODO
    functions[*name] = fs;
  }
};

class CC{
public:
  BC global;
  std::vector<BC *> blocks;

  BC* getBlock(){
    if(blocks.size()!=0)
      return blocks.back();
    return &global;
  }

  BC* getParentBlock(){
    if(blocks.size() < 2)
      return &global;
    return blocks[blocks.size()-2];
  }

  Type *getVariableType(std::string *name){
    for(auto i = blocks.rbegin(); i!=blocks.rend(); i++){
      auto vars = (*i)->vars;
      auto p = vars.find(*name);
      if(p!=vars.end())
        return p->second;
    }
    return global.vars[*name];
  }

  StructStatement *getStruct(std::string *name){
    for(auto i = blocks.rbegin(); i!=blocks.rend(); i++){
      auto vars = (*i)->structs;
      auto p = vars.find(*name);
      if(p!=vars.end())
        return p->second;
    }
    return global.structs[*name];
  }

  FunctionSignature *getFunction(std::string *name){
    for(auto i = blocks.rbegin(); i!=blocks.rend(); i++){
      auto vars = (*i)->functions;
      auto p = vars.find(*name);
      if(p!=vars.end())
        return p->second;
    }
    return global.functions[*name];
  }
};

void check_health(Statement *stmts, CC* cc);
Type *resolveType(Expression *exp, CC *cc);

void check_health(std::vector<Statement *> *stmts){
  // TODO

  CC *cc = new CC();
  
  for(auto s: *stmts){
    check_health(s, cc);
  }
}

std::vector<Statement *>* compile(char *fileName){
  yyin = fopen(fileName, "r");
  extern std::vector<Statement *> *mainProgramNode;
  yyparse();

  check_health(mainProgramNode);

  return mainProgramNode;
}

void functionSignHealth(FunctionSignature *fs, CC *cc){
  BC *b = cc->getBlock();
  b->returnType = fs->returnT;

  for(auto arg: *fs->args){
    if(!arg->vardiac)
      b->newVar(arg->name, arg->t);
    // else TODO do when we have arrays
  }

  cc->getParentBlock()->newFunction(fs->name, fs);
}

void functionDefineHealth(FunctionDefine *fd, CC *cc){
  cc ->blocks.push_back(new BC());

  functionSignHealth(fd->sign, cc);

  for(auto s: *fd->body->stmts){
    check_health(s, cc);
  }

  cc->blocks.pop_back();
}

void returnStmtHealth(ReturnStatement *rs, CC *cc){
  // TODO handle void?

  // health expr
  check_health(rs->exp, cc);

  // return type
  Type *t = resolveType(rs->exp, cc);
  Type *ft = cc->getBlock()->returnType;

  auto c = ft->compatible(t);
  if(c == Compatibility::UNCOMPATIBLE ){
    printf("Uncompatible type for return statement at line %d\n Cant convert %s to %s\n",
           rs->lineno,
           t->displayName().c_str(),
           ft->displayName().c_str());
    exit(1);
  }

  if(c == Compatibility::ExpCast){
    printf("Uncompatible type for return statement at line %d\n Cant convert %s to %s safely\n",
           rs->lineno,
           t->displayName().c_str(),
           ft->displayName().c_str());
    exit(1);
  }
}

void structCheckHealth(StructStatement *ss, CC *cc){
  cc->getBlock()->newStruct(ss->name, ss);
  // TODO
}

void variableDeclHealthCheck(VariableDecl *vd,CC *cc){
  Type *type;
  // TODO check exp type
  if(vd->exp){
    if(vd->t){
      auto etype = resolveType(vd->exp, cc);
      auto c = vd->t->compatible(etype);
      if(c==OK || c==ImpCast){
        delete etype;
      } else {
        printf("Uncompatible type covnersion from %s to %s at line %d\n",
               etype->displayName().c_str(), vd->t->displayName().c_str(), vd->lineno);
        exit(1);
      }
    } else {
      type = resolveType(vd->exp, cc);
    }
  } else {
    type = vd->t;
  }

  // Add to block
  cc->getBlock()->newVar(vd->name, type);
}

void check_health(Statement *stmt, CC *cc){
  auto t = typeid(*stmt).hash_code();

  // TODO check resolve_type if it's an expression.
  // TODO this shouldn't be here if we call the resolveType of every expression ourselves.
  if(Expression *exp = dynamic_cast<Expression*>(stmt)){
    resolveType(exp, cc);
  }

  if(t == typeid(FunctionDefine).hash_code())
    return functionDefineHealth((FunctionDefine*) stmt, cc);

  if(t == typeid(ReturnStatement).hash_code())
    return returnStmtHealth((ReturnStatement*) stmt, cc);

  if(t == typeid(FunctionSignature).hash_code()){
    // TODO memory leak
    cc->blocks.push_back(new BC());
    functionSignHealth((FunctionSignature *) stmt, cc);
    cc->blocks.pop_back();
    return;
  }

  if(t == typeid(FunctionCallExpr).hash_code()){
    // TODO
    auto *fce= (FunctionCallExpr*) stmt;
    for(auto e: *fce->expr)
      check_health(e, cc);
    return;
  }

  if(t == typeid(StructStatement).hash_code()){
    structCheckHealth((StructStatement*)stmt, cc);
    // TODO
    return;
  }

  if(t == typeid(IfStatement).hash_code()){
    // TODO
    IfStatement *is = (IfStatement *) stmt;
    check_health(is->exp, cc);
    check_health(is->i, cc);
    if(is->e)
      check_health(is->e, cc);
    return;
  }

  if(t == typeid(VariableDecl).hash_code()){
    return variableDeclHealthCheck((VariableDecl*) stmt, cc);
  }

  if(t == typeid(VariableAssign).hash_code()){
    VariableAssign *va = (VariableAssign*) stmt;
    check_health(va->base, cc);
    check_health(va->exp, cc);
    // TODO
    return;
  }

  if(t == typeid(WhileStatement).hash_code()){
    // TODO
    WhileStatement *ws = (WhileStatement*) stmt;
    check_health(ws->exp, cc);
    check_health(ws->w, cc);
    return;
  }

  if(t == typeid(CodeBlock).hash_code()){
    CodeBlock *cb = (CodeBlock *)stmt;
    for(auto s: *cb->stmts){
      check_health(s, cc);
    }

    return;
  }

  if(t == typeid(PointerAccessExpr).hash_code()){
    // TODO
    PointerAccessExpr *pae = (PointerAccessExpr*)stmt;
    check_health(pae->exp, cc);
    return;
  }

  if(t == typeid(IntValue).hash_code()){
    // TODO
    return;
  }

  if(t == typeid(StringValue).hash_code()){
    // TODO
    return;
  }

  if(t == typeid(PointerExpr).hash_code()){
    // TODO
    return;
  }

  if(t == typeid(MemberExpr).hash_code()){
    MemberExpr *me = (MemberExpr*) stmt;
    check_health(me->e, cc);
    check_health(me->e, cc);
    // TODO check to see if mem is in the struct
    return;
  }

  if(t == typeid(CastExpr).hash_code()){
    // TODO
    CastExpr *ce = (CastExpr*) stmt;
    check_health(ce->exp, cc);
    return;
  }

  if(t == typeid(VariableExpr).hash_code()){
    // TODO check to see if var is defined.
    // TODO
    return;
  }

  if(t == typeid(BinaryOperation).hash_code()){
    auto bo = (BinaryOperation *) stmt;
    check_health(bo->lhs, cc);
    check_health(bo->rhs, cc);
    // TODO check to see if the op is valid
    // TODO
    return;
  }

  printf("Undefined check_health for statement %s at compiler::check_health, lineno: %d\n", typeid(*stmt).name(), stmt->lineno);
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
    auto t = (StructType*) resolveType(me->e, cc);
    auto ss = cc->getStruct(t->name);

    for(auto m: *ss->members){
      if(m->name->compare(*me->mem) == 0){
        expr->exprType = m->t->clone();
        return expr->exprType;
      }
    }
    printf("Member type not found, memberExprType on line %d\n", me->lineno);
    exit(1);
  }

  if(t == typeid(FunctionCallExpr).hash_code()){
    FunctionCallExpr *fce = (FunctionCallExpr *) expr;
    // TODO check for clone
    auto f = cc->getFunction(fce->name);
    if(!f){
      printf("Unknown function %s at line %d\n",
             fce->name->c_str(), fce->lineno);
      exit(1);
    }
    expr->exprType = f->returnT->clone();
    return expr->exprType;
  }

  printf("Undefined resolveType for expression %s at compiler::resolveType\n", typeid(*expr).name());
  exit(1);
}
