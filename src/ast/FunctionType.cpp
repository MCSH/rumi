#include "FunctionType.h"

// TODO compile & prep

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
Compatibility FunctionType::compatible(Type *t){
  return INCOMPATIBLE;
}

void *FunctionType::castgen(CC *cc, Expression *e){
  return 0;
}
