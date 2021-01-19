#pragma once
#include "Expression.h"

class PreOp: public Expression{
 public:
  Expression *value;
  std::string op;

  PreOp(std::string op, Expression *value);
  
  virtual void compile(CC *cc);
  virtual void prepare(CC *cc);
  virtual void* exprgen(CC *cc);
  virtual Type *type(CC *cc);
  virtual void *allocagen(CC *cc);
};
