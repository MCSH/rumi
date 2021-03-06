#pragma once
#include "Expression.h"

class BinOp: public Expression{
 public:
  Expression *lhs, *rhs;
  std::string op;

  BinOp(Expression *lhs, std::string op, Expression *rhs);

  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;
  virtual void* exprgen(CC *cc) override;
  virtual Type *type(CC *cc) override;
  virtual void *allocagen(CC *cc) override;
};
