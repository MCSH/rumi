#pragma once
#include "../Type.h"

class VoidType: public Type{
  virtual Compatibility compatible(Type *t);

  virtual std::string displayName();

  virtual VoidType* clone();

  virtual llvm::Type *typeGen(Context *cc);
};
