#pragma once
#include "Statement.h"
#include "Expression.h"

class PtrAssign: public Statement{
 public:
  Expression *ptr, *value;

  PtrAssign(Expression *ptr, Expression *value);

  virtual void prepare(CC *cc) override;
  virtual void compile(CC *cc) override;
  virtual void codegen(CC *cc) override;
};
