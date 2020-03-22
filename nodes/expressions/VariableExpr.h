#pragma once
#include "../Expression.h"

class VariableExpr: public Expression{
public:
  std::string *name;
  VariableExpr(std::string *n): name(n){
    
  }

  virtual ~VariableExpr();
  virtual llvm::Value *exprGen(CodegenContext *cc);
  virtual llvm::Value *getAlloca(CodegenContext *cc);
  virtual void compile(Context *cc);
  virtual Type *resolveType(Context *cc);
};
