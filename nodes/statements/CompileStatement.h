#pragma once
#include "../Statement.h"
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <string>

class CompileStatement: public Statement{
public:
  std::string *name;
  Statement *s;
  CompileStatement(std::string *n, Statement *s): name(n), s(s){}
  virtual ~CompileStatement();
  virtual void codegen(Context *cc);
  virtual void compile(Context *cc);
};

void import_compiler(llvm::ExecutionEngine *EE, Context *cc);
