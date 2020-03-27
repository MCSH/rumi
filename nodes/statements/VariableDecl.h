#pragma once
#include "../Statement.h"
#include "../Expression.h"

class VariableDecl: public Statement{
public:
  std::string *name=0;
  Type *t=0;
  Expression *exp=0;
  VariableDecl(){}
  VariableDecl(std::string *n, Type *t, Expression *e=nullptr): name(n), t(t), exp(e){}

  virtual ~VariableDecl();
  virtual void codegen(Context *cc);
  virtual void compile(Context *cc);
};