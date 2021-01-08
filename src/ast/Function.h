#pragma once
#include "Statement.h"
#include "ast.h"
#include <vector>

class Args;
class Type;

class Function: public Statement{
  std::vector<Statement *> statements;
  std::vector<Args *> args;
  Type * returnType;

  virtual void compile(CC *cc);
  virtual void prepeare(CC *cc);
  virtual void codegen(CC *cc);
};
