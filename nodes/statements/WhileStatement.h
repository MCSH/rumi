#pragma once
#include "../Expression.h"
#include "../Statement.h"

class WhileStatement: public Statement{
public:
  Statement *w;
  Expression *exp;

  WhileStatement(Expression *exp, Statement *w): w(w), exp(exp){
  }

  virtual ~WhileStatement();
  virtual void codegen(CodegenContext *cc);
  virtual void compile(Context *cc);
};
