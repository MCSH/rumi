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

  virtual void prepeare(CC *cc) override;
  virtual void compile(CC *cc) override;
  virtual void codegen(CC *cc) override;
};

