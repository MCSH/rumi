#pragma once
#include "../Statement.h"
#include "../Expression.h"
#include "../../BlockContext.h"
#include <llvm/IR/BasicBlock.h>

class IfContext: public BlockContext{};

class IfStatement: public Statement{
public:
  Statement *i, *e;
  IfContext *ib=0, *eb=0;
  Expression *exp;

  IfStatement(Expression *exp, Statement *i, Statement *e=nullptr): i(i), e(e), exp(exp){
  }

  virtual ~IfStatement();
  virtual void codegen(Context *cc);
  virtual void compile(Context *cc);

  IfContext* getIB(llvm::BasicBlock *bb=0);
  IfContext* getEB(llvm::BasicBlock *bb=0);
};
