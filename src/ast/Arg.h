#pragma once
#include "Type.h"
#include "Statement.h"

class Arg: public Statement{
 public:
  std::string id;
  Type *type;

  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;
  virtual void codegen(CC *cc) override;
};
