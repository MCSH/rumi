#pragma once
#include "../Expression.h"

class TypeNode: public Expression{
public:
  Type *exprType;
  TypeNode(Type *exprType): exprType(exprType){}
  virtual llvm::Value *exprGen(CodegenContext *cc){}
  virtual llvm::Value *getAlloca(CodegenContext *cc){}
  virtual void compile(Context *cc){}
  virtual Type *resolveType(Context *cc){}
};
