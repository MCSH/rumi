#pragma once
#include "../Statement.h"
#include "../../BlockContext.h"
#include <vector>

class CodeBlock: public Statement, public BlockContext{
public:
  std::vector<Statement *> *stmts;
  CodeBlock(std::vector<Statement*> *s): stmts(s){}

  virtual ~CodeBlock();
  virtual void codegen(Context *cc);
  virtual void compile(Context *cc);
};
