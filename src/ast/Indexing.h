#pragma once
#include "Expression.h"

// TODO expose in compiler API

class Indexing: public Expression{
public:
  Expression *expr, *index;
  Indexing(Expression *expr, Expression *index);

  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;
  virtual void* exprgen(CC *cc) override;
  virtual Type *type(CC *cc) override;
  virtual void *allocagen(CC *cc) override;
};
