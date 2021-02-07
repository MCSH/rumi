#pragma once
#include "Expression.h"

class ConstFloat: public Expression{
 public:
  double value;
  bool is32;
  ConstFloat(double v, bool is32);

  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;
  virtual void* exprgen(CC *cc) override;
  virtual Type* type(CC *cc) override;
  virtual void* allocagen(CC *cc) override;
};
