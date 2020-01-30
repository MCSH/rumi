#pragma once
#include "../Statement.h"
#include "../Expression.h"

class VariableAssign: public Statement{
public:
  Expression *base;
  Expression *exp;
  VariableAssign(Expression *b, Expression *exp): base(b), exp(exp){
  }

  virtual ~VariableAssign();
  virtual void codegen(CodegenContext *cc);
  virtual void compile(CompileContext *cc);
};
