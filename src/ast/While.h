#pragma once
#include "Statement.h"
#include "Expression.h"

class While: public Statement{
 public:
  Expression *condition;
  Statement *st;

  While(Expression *condition, Statement *st);

  virtual void prepare(CC *cc) override;
  virtual void compile(CC *cc) override;
  virtual void codegen(CC *cc) override;
};
