#pragma once
#include "Expression.h"

class PtrValue: public Expression{
 public:
  Expression *ptr;
  
  PtrValue(Expression *ptr);

  virtual void compile(CC *cc);
  virtual void prepare(CC *cc);
  virtual void* exprgen(CC *cc);
  virtual Type *type(CC *cc);
  virtual void *allocagen(CC *cc);
};
