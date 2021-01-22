#pragma once
#include "Statement.h"
#include "Expression.h"

class Return: public Statement{
 public:
  Expression *value;

  Return(Expression *value);

  virtual void set(std::string key, void *value) override;
  virtual void add(std::string key, void *value) override;

  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;
  virtual void codegen(CC *cc) override;
};
