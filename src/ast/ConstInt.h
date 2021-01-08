#pragma once
#include "Expression.h"

class ConstInt: public Expression{
 public:
  long long value;
  ConstInt(long long v);

  virtual void compile(CC *cc);
  virtual void prepeare(CC *cc);
  virtual void* exprgen(CC *cc);
};
