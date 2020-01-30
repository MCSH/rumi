#pragma once
#include "../Expression.h"

class PointerExpr: public Expression{
public:
  Expression *exp;
  PointerExpr(Expression *e):exp(e){}

  virtual ~PointerExpr();
  virtual llvm::Value *exprGen(CodegenContext *cc);
  virtual llvm::Value *getAlloca(CodegenContext *cc);
  virtual void compile(CompileContext *cc);
  virtual Type *resolveType(CompileContext *cc);
};
