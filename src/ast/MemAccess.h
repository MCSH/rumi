#pragma once
#include "Expression.h"

class MemAccess: public Expression{
 public:
  Expression *exp;
  std::string id;
  
  MemAccess(Expression *exp, std::string id);

  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;
  virtual void* exprgen(CC *cc) override;
  virtual Type *type(CC *cc) override;
  virtual void *allocagen(CC *cc) override;
};
