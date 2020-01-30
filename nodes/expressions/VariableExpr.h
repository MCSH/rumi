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
  virtual void compile(CompileContext *cc);
  virtual Type *resolveType(CompileContext *cc);
};
