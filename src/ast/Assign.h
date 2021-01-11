#pragma once
#include "Statement.h"
#include <string>

class Type;
class Expression;

class Assign: public Statement{
 public:
  Expression *baseExpr;
  Expression *expression;

  Assign(Expression *baseExpr, Expression *e);

  bool casting;

  virtual void prepare(CC *cc) override;
  virtual void compile(CC *cc) override;
  virtual void codegen(CC *cc) override;
};
