#pragma once
#include "Statement.h"
#include "Expression.h"

// TODO expose to compiler API

class Using: public Statement{
public:
  Expression *e;
  Using(Expression *e);

  virtual void prepare(CC *cc) override;
  virtual void compile(CC *cc) override;
  virtual void codegen(CC *cc) override;
};
