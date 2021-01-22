#pragma once
#include "Statement.h"
#include <string>

class Type;
class Expression;

class Define: public Statement{
 public:
  std::string id;
  Type *type = 0;
  Expression *expression = 0;

  Define(std::string id, Expression *e, Type *t);

  virtual void set(std::string key, void *value) override;
  virtual void add(std::string key, void *value) override;

  virtual void prepare(CC *cc) override;
  virtual void compile(CC *cc) override;
  virtual void codegen(CC *cc) override;
};

