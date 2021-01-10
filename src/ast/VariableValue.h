#pragma once
#include "Expression.h"

class VariableValue: public Expression{
 public:
  std::string id;
  
  VariableValue(std::string id);

  virtual void compile(CC *cc);
  virtual void prepare(CC *cc);
  virtual void* exprgen(CC *cc);
  virtual Type *type(CC *cc);
};
