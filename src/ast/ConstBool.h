#pragma once
#include "Expression.h"

class ConstBool: public Expression{
 public:
  bool truth;
  ConstBool(bool truth);

  virtual void compile(CC *cc);
  virtual void prepare(CC *cc);
  virtual void* exprgen(CC *cc);
  virtual Type* type(CC *cc);
  virtual void* allocagen(CC *cc);
};
