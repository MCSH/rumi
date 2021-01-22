#pragma once
#include "Expression.h"

class VariableValue: public Expression{
 public:
  std::string id;
  
  VariableValue(std::string id);

  virtual void set(std::string key, void *value) override;
  virtual void add(std::string key, void *value) override;

  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;
  virtual void* exprgen(CC *cc) override;
  virtual Type *type(CC *cc) override;
  virtual void *allocagen(CC *cc) override;
};
