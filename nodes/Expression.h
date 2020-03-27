#pragma once
#include "Type.h"
#include "Statement.h"

#include <cstdlib>

#include <llvm/IR/Value.h>

class Expression: public Statement{
public:
  Type *exprType = 0;
  virtual llvm::Value *exprGen(Context *cc)=0;
  virtual llvm::Value *getAlloca(Context *cc)=0; // TODO maybe extend Expression for this because most don't have it
  virtual void codegen(Context *cc){} // Almost all of expressions have empty codegen
  virtual Type *resolveType(Context *cc)=0;
};