#pragma once
#include "../Expression.h"

class IntValue: public Expression{
public:
  std::string *val=0;
  int size;

  IntValue(std::string *val): val(val){}
  IntValue(int size): size(size){}

  virtual ~IntValue();
  virtual llvm::Value *exprGen(Context *cc);
  virtual llvm::Value *getAlloca(Context *cc);
  virtual void compile(Context *cc);
  virtual Type *resolveType(Context *cc);
};
