#pragma once
#include "../Expression.h"

class CastExpr: public Expression{
public:
  Type *t;
  Expression *exp;
  CastExpr(Type *t, Expression *e): t(t), exp(e){
  }

  virtual ~CastExpr();
  virtual llvm::Value *exprGen(CodegenContext *cc);
  virtual llvm::Value *getAlloca(CodegenContext *cc);
  virtual void compile(Context *cc);
  virtual Type *resolveType(Context *cc);
};
