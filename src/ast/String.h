#pragma once
#include "Expression.h"

class String: public Expression{
 public:
  std::string val;
  String(std::string val);

  virtual void compile(CC *cc);
  virtual void prepare(CC *cc);
  virtual void* exprgen(CC *cc);
  virtual Type* type(CC *cc);
};
