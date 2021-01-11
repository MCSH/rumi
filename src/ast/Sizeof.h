#pragma once
#include "Expression.h"

class Sizeof: public Expression{
 public:
  Type *type_;
  Sizeof(Type *type_);

  virtual void compile(CC *cc);
  virtual void prepare(CC *cc);
  virtual void* exprgen(CC *cc);
  virtual Type* type(CC *cc);
  virtual void* allocagen(CC *cc);
};
