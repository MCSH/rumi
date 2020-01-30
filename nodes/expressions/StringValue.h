#pragma once
#include "../Expression.h"

class StringValue: public Expression{
public:
  std::string *val;
  StringValue(std::string *v);
  virtual ~StringValue();
  virtual llvm::Value *exprGen(CodegenContext *cc);
  virtual llvm::Value *getAlloca(CodegenContext *cc);
  virtual void compile(CompileContext *cc);
  virtual Type *resolveType(CompileContext *cc);
};
