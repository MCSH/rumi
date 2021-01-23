#pragma once
#include "Statement.h"
#include "ast.h"
#include <vector>

class Arg;
class Type;
class BlockContext;

class Function: public Statement{
public:
  std::string id;
  std::vector<Statement *> statements;
  std::vector<Arg *> args;
  Type * returnType;

  BlockContext *b;

  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;
  virtual void codegen(CC *cc) override;
};
