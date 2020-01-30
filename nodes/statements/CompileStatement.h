#pragma once
#include "../Statement.h"
#include <string>

class CompileStatement: public Statement{
public:
  std::string *name;
  Statement *s;
  CompileStatement(std::string *n, Statement *s): name(n), s(s){}
  virtual ~CompileStatement();
  virtual void codegen(CodegenContext *cc);
  virtual void compile(CompileContext *cc);
};
