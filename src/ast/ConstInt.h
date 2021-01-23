#pragma once
#include "Expression.h"

class ConstInt: public Expression{
 public:
  long long value;
  ConstInt(long long v);

  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;
  virtual void* exprgen(CC *cc) override;
  virtual Type* type(CC *cc) override;
  virtual void* allocagen(CC *cc) override;
};
