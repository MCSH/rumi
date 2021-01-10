#pragma once
#include "Statement.h"
#include "Expression.h"

class If: public Statement{
 public:
  Expression *condition;
  Statement *st1, *st2;

  If(Expression *condition, Statement *st1, Statement *st2);

  virtual void prepare(CC *cc) override;
  virtual void compile(CC *cc) override;
  virtual void codegen(CC *cc) override;
};
