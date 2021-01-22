#pragma once
#include "Expression.h"

class PreOp: public Expression{
 public:
  Expression *value;
  std::string op;

  PreOp(std::string op, Expression *value);
  
  virtual void set(std::string key, void *value) override;
  virtual void add(std::string key, void *value) override;

  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;
  virtual void* exprgen(CC *cc) override;
  virtual Type *type(CC *cc) override;
  virtual void *allocagen(CC *cc) override;
};
