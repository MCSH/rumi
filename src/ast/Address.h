#pragma once
#include "Expression.h"

class Address: public Expression{
 public:
  Expression *exp;

  Address(Expression *);

  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;
  virtual void* exprgen(CC *cc) override;
  virtual Type *type(CC *cc) override;
  virtual void *allocagen(CC *cc) override;
};
