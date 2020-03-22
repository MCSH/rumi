#pragma once
#include "../Expression.h"

class StringValue: public Expression{
public:
  std::string *val;
  StringValue(std::string *v);
  virtual ~StringValue();
  virtual llvm::Value *exprGen(Context *cc);
  virtual llvm::Value *getAlloca(Context *cc);
  virtual void compile(Context *cc);
  virtual Type *resolveType(Context *cc);
};
