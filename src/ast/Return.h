#pragma once
#include "Statement.h"
#include "Expression.h"

class Return: public Statement{
 public:
  Expression *value;

  Return(Expression *value);

  virtual void compile(CC *cc);
  virtual void prepare(CC *cc);
  virtual void codegen(CC *cc);
};
