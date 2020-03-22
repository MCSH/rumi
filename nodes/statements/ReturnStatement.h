#pragma once
#include "../Statement.h"
#include "../Expression.h"

class ReturnStatement: public Statement{
public:
  Expression *exp;
  ReturnStatement(Expression *e): exp(e){}

  ReturnStatement(){
    exp = 0;
  }

  virtual ~ReturnStatement(){
    delete exp;
  }
  virtual void codegen(Context *cc);
  virtual void compile(Context *cc);
};
