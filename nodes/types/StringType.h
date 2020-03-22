#pragma once
#include "../Type.h"

class StringType: public Type{
  virtual Compatibility compatible(Type *t);

  virtual std::string displayName();

  virtual StringType* clone();
  virtual llvm::Type *typeGen(Context *cc);
};
