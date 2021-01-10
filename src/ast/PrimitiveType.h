#pragma once
#include "Type.h"
#include "../TypeEnum.h"

class PrimitiveType: public Type{
public:
  TypeEnum key;
  PrimitiveType(TypeEnum key);
  virtual void* typegen(CC *cc);
  virtual void compile(CC *cc);
  virtual void prepare(CC *cc);
  bool hasOp(CC *cc, std::string op, Expression *rhs);
  void* opgen(CC *cc, Expression *lhs, std::string op, Expression *rhs);
  Type* optyperesolve(CC *cc, std::string op, Expression *rhs);
  Compatibility compatible(Type *t);
  Expression* castFrom(Expression *e);
};
