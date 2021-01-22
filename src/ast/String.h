#pragma once
#include "Expression.h"

class String: public Expression{
 public:
  std::string val;
  String(std::string val);

  virtual void set(std::string key, void *value) override;
  virtual void add(std::string key, void *value) override;

  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;
  virtual void* exprgen(CC *cc) override;
  virtual Type* type(CC *cc) override;
  virtual void *allocagen(CC *cc) override;
};
