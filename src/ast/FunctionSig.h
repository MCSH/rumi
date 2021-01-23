#pragma once
#include "Statement.h"
#include "ast.h"
#include <vector>

class Arg;
class Type;

class FunctionSig: public Statement{
public:
  std::vector<Arg *> args;
  Type * returnType;
  bool vararg;

  std::string id;

  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;
  virtual void codegen(CC *cc) override;
};
