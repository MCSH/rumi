#pragma once
#include "../Statement.h"

class DeferStatement: public Statement{
public:
  Statement *s;

  DeferStatement(Statement *s): s(s){
  }

  virtual ~DeferStatement();
  virtual void codegen(CodegenContext *cc);
  virtual void compile(CompileContext *cc);
};
