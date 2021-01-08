#pragma once
#include "Type.h"
#include "../TypeEnum.h"

class PrimitiveType: public Type{
public:
  TypeEnum key;
  PrimitiveType(TypeEnum key);
  virtual void* typegen(CC *cc);
  virtual void compile(CC *cc);
  virtual void prepeare(CC *cc);
};
