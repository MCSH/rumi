#pragma once
#include "Expression.h"

class Sizeof: public Expression{
 public:
  Type *type_;
  Sizeof(Type *type_);

  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;
  virtual void* exprgen(CC *cc) override;
  virtual Type* type(CC *cc) override;
  virtual void* allocagen(CC *cc) override;
};
