#pragma once
#include "Statement.h"
#include <string>

class Type;
class Expression;

class Assign: public Statement{
 public:
  std::string id;
  Expression *expression = 0;

  Assign(std::string id, Expression *e);

  bool casting;

  virtual void prepare(CC *cc) override;
  virtual void compile(CC *cc) override;
  virtual void codegen(CC *cc) override;
};
