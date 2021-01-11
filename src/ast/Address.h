#pragma once
#include "Expression.h"

class Address: public Expression{
 public:
  Expression *exp;

  Address(Expression *);

  virtual void compile(CC *cc);
  virtual void prepare(CC *cc);
  virtual void* exprgen(CC *cc);
  virtual Type *type(CC *cc);
  virtual void *allocagen(CC *cc);
};
