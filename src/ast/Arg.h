#pragma once
#include "Type.h"
#include "Statement.h"

class Arg: public Statement{
 public:
  std::string id;
  Type *type;

  virtual void compile(CC *cc);
  virtual void prepeare(CC *cc);
  virtual void codegen(CC *cc);
};
