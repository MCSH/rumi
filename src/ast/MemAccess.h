#pragma once
#include "Expression.h"

class MemAccess: public Expression{
 public:
  Expression *exp;
  std::string id;
  
  MemAccess(Expression *exp, std::string id);

  virtual void compile(CC *cc);
  virtual void prepare(CC *cc);
  virtual void* exprgen(CC *cc);
  virtual Type *type(CC *cc);
  virtual void *allocagen(CC *cc);
};
