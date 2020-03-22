#pragma once
#include "../Expression.h"

class ArrayExpr: public Expression{
public:
  Expression *e;
  Expression *mem;
  ArrayExpr(Expression *e, Expression *mem): mem(mem), e(e){}

  virtual ~ArrayExpr();
  virtual llvm::Value *exprGen(Context *cc);
  virtual llvm::Value *getAlloca(Context *cc);
  virtual void compile(Context *cc);
  virtual Type *resolveType(Context *cc);
};
