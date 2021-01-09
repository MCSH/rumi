#pragma once
#include "Expression.h"

class VariableValue: public Expression{
 public:
  std::string id;
  
  VariableValue(std::string id);

  virtual void compile(CC *cc);
  virtual void prepeare(CC *cc);
  virtual void* exprgen(CC *cc);
};
