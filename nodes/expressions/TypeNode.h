#pragma once
#include "../Expression.h"

class TypeNode: public Expression{
public:
  Type *exprType;
  TypeNode(Type *exprType): exprType(exprType){}
  virtual llvm::Value *exprGen(CodegenContext *cc){}
  virtual llvm::Value *getAlloca(CodegenContext *cc){}
  virtual void compile(CompileContext *cc){}
  virtual Type *resolveType(CompileContext *cc){}
};
