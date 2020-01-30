#pragma once
#include "../Statement.h"
#include "../Expression.h"

class IfStatement: public Statement{
public:
  Statement *i, *e;
  Expression *exp;

  IfStatement(Expression *exp, Statement *i, Statement *e=nullptr): i(i), e(e), exp(exp){
  }

  virtual ~IfStatement();
  virtual void codegen(CodegenContext *cc);
  virtual void compile(CompileContext *cc);
};
