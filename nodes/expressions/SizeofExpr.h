#pragma once
#include "../Expression.h"

class SizeofExpr: public Expression{
public:
  Type *t;

  SizeofExpr(Type *t): t(t){}

  virtual ~SizeofExpr();
  virtual llvm::Value *exprGen(CodegenContext *cc);
  virtual llvm::Value *getAlloca(CodegenContext *cc);
  virtual void compile(Context *cc);
  virtual Type *resolveType(Context *cc);
};
