#include "FunctionType.h"

void* FunctionType::typegen(CC *cc){
  return 0;
}

void FunctionType::compile(CC *cc){
}

void FunctionType::prepeare(CC *cc){
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
