#pragma once
#include "../Expression.h"

class PointerAccessExpr: public Expression{
public:
  Expression *exp;
  PointerAccessExpr(Expression *e):exp(e){}

  virtual ~PointerAccessExpr();
  virtual llvm::Value *exprGen(CodegenContext *cc);
  virtual llvm::Value *getAlloca(CodegenContext *cc);
  virtual void compile(Context *cc);
  virtual Type *resolveType(Context *cc);
};
