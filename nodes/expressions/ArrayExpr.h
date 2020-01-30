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
  virtual void compile(CompileContext *cc);
  virtual Type *resolveType(CompileContext *cc);
};
