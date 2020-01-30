#pragma once
#include "../Statement.h"
#include <vector>

class CodeBlock: public Statement{
public:
  std::vector<Statement *> *stmts;
  CodeBlock(std::vector<Statement*> *s): stmts(s){}

  virtual ~CodeBlock();
  virtual void codegen(CodegenContext *cc);
  virtual void compile(CompileContext *cc);
};
