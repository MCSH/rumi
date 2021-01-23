#pragma once
#include "Expression.h"

class String: public Expression{
 public:
  std::string val;
  String(std::string val);

  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;
  virtual void* exprgen(CC *cc) override;
  virtual Type* type(CC *cc) override;
  virtual void *allocagen(CC *cc) override;
};
