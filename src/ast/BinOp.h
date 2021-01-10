#pragma once
#include "Expression.h"

class BinOp: public Expression{
 public:
  Expression *lhs, *rhs;
  std::string op;

  BinOp(Expression *lhs, std::string op, Expression *rhs);
  
  virtual void compile(CC *cc);
  virtual void prepare(CC *cc);
  virtual void* exprgen(CC *cc);
  virtual Type *type(CC *cc);
};
