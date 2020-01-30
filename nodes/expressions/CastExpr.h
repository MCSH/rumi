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
  virtual void compile(CompileContext *cc);
  virtual Type *resolveType(CompileContext *cc);
};
