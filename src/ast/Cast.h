#pragma once
#include "Expression.h"

class Cast: public Expression{
 public:
  Type *type_;
  Expression *exp;

  Cast(Expression *exp, Type *type_);

  virtual void compile(CC *cc);
  virtual void prepare(CC *cc);
  virtual void *exprgen(CC *cc);
  virtual Type *type(CC *cc);
  virtual void *allocagen(CC *cc);
};
