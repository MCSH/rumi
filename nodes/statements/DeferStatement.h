#pragma once
#include "../Statement.h"

class DeferStatement: public Statement{
public:
  Statement *s;

  DeferStatement(Statement *s): s(s){
  }

  virtual ~DeferStatement();
  virtual void codegen(Context *cc);
  virtual void compile(Context *cc);
};
