#pragma once
#include "../Expression.h"

class SizeofExpr: public Expression{
public:
  Type *t;

  SizeofExpr(Type *t): t(t){}

  virtual ~SizeofExpr();
  virtual llvm::Value *exprGen(CodegenContext *cc);
  virtual llvm::Value *getAlloca(CodegenContext *cc);
  virtual void compile(CompileContext *cc);
  virtual Type *resolveType(CompileContext *cc);
};
