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

  virtual void compile(CC *cc);
  virtual void prepeare(CC *cc);
  virtual void codegen(CC *cc);

  void setId(std::string id);
};
