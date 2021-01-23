#pragma once
#include "Expression.h"

class Cast: public Expression{
 public:
  Type *type_;
  Expression *exp;

  Cast(Expression *exp, Type *type_);

  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;
  virtual void *exprgen(CC *cc) override;
  virtual Type *type(CC *cc) override;
  virtual void *allocagen(CC *cc) override;
};
