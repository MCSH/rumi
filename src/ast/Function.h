#pragma once
#include "Statement.h"
#include "ast.h"
#include <vector>

class Arg;
class Type;
class BlockContext;

class Function: public Statement{
public:
  std::vector<Statement *> statements;
  std::vector<Arg *> args;
  Type * returnType;

  BlockContext *b;

  std::string id;

  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;
  virtual void codegen(CC *cc) override;

  void setId(std::string id);
};
