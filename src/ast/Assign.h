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

  virtual void set(std::string key, void *value) override;
  virtual void add(std::string key, void *value) override;

  virtual void prepare(CC *cc) override;
  virtual void compile(CC *cc) override;
  virtual void codegen(CC *cc) override;
};
