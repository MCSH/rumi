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

  std::string id;

  virtual void compile(CC *cc);
  virtual void prepare(CC *cc);
  virtual void codegen(CC *cc);

  void setId(std::string id);
};
