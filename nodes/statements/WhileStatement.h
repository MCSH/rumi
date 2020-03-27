#pragma once
#include "../Expression.h"
#include "../Statement.h"
#include "../../BlockContext.h"

class WhileBC: public BlockContext{};

class WhileStatement: public Statement, public BlockContext{
public:
  Statement *w;
  Expression *exp;
  WhileBC cb;

  WhileStatement(Expression *exp, Statement *w): w(w), exp(exp){
  }

  virtual ~WhileStatement();
  virtual void codegen(Context *cc);
  virtual void compile(Context *cc);
};
