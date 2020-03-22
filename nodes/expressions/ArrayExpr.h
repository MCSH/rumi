#pragma once
#include "../Expression.h"

class ArrayExpr: public Expression{
public:
  Expression *e;
  Expression *mem;
  ArrayExpr(Expression *e, Expression *mem): mem(mem), e(e){}

  virtual ~ArrayExpr();
  virtual llvm::Value *exprGen(CodegenContext *cc);
  virtual llvm::Value *getAlloca(CodegenContext *cc);
  virtual void compile(Context *cc);
  virtual Type *resolveType(Context *cc);
};
