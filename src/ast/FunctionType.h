#pragma once
#include "Type.h"
#include <vector>

class FunctionType: public Type{
public:

  std::vector<Type *> args;
  Type *returnType;

  virtual void* typegen(CC *cc);
  virtual void compile(CC *cc);
  virtual void prepare(CC *cc);
  bool hasOp(CC *cc, std::string op, Expression *rhs);
  void* opgen(CC *cc, Expression *lhs, std::string op, Expression *rhs);
  Type* optyperesolve(CC *cc, std::string op, Expression *rhs);
  Compatibility compatible(Type *t);
  void* castgen(CC *cc, Expression *e);
};
