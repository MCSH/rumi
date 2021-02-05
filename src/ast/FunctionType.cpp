#include "FunctionType.h"

// TODO compile & prep & typegen

void* FunctionType::typegen(CC *cc){
  return 0;
}

void FunctionType::compile(CC *cc){
}

void FunctionType::prepare(CC *cc){
}

bool FunctionType::hasOp(CC *cc, std::string op, Expression *rhs){
  return false;
}

void* FunctionType::opgen(CC *cc, Expression *lhs, std::string op, Expression *rhs){
  return 0;
}

Type* FunctionType::optyperesolve(CC *cc, std::string op, Expression *rhs){
  return 0;
}
Compatibility FunctionType::compatible(CC *cc, Type *t){
  return INCOMPATIBLE;
}

void *FunctionType::castgen(CC *cc, Expression *e){
  return 0;
}

bool FunctionType::hasMem(CC *cc, Expression *exp, std::string id){
  return false;
}

void *FunctionType::memgen(CC *cc, Expression *exp, std::string id){
  return 0;
}

void *FunctionType::memalloca(CC *cc, Expression *exp, std::string id){
  return 0;
}

Type *FunctionType::memtyperesolve(CC *cc, Expression *exp, std::string id){
  return 0;
}

bool FunctionType::hasPreOp(CC *cc, std::string op){
  return false;
}

Type *FunctionType::preoptyperesolve(CC *cc, std::string op){
  return 0;
}

void *FunctionType::preopgen(CC *cc, std::string op, Expression *value){
  return 0;
}

std::string FunctionType::toString(){
  std::string ans = "(";
  bool first = true;
  for (auto t : args) {
    if(first) ans += t->toString();
    else ans += ", " + t->toString();
    first = false;
  }
  if(vararg) ans += "...";
  ans += ")->" + (returnType?returnType->toString():"unit");
  return ans;
}


void FunctionType::initgen(CC *cc, Expression *alloca){
  // Do nothing
}

bool FunctionType::hasIndex(CC *cc, Expression *index){
  return 0;
}

void *FunctionType::indexgen(CC *cc, Expression *expr, Expression *index){
  return 0;
}

Type *FunctionType::indextyperesolve(CC *cc, Expression *index){
  return 0;
}

void *FunctionType::indexallocagen(CC *cc, Expression *expr, Expression *index){
  return 0;
}
