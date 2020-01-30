#pragma once
#include "../Type.h"

class AnyType: public Type{
  virtual Compatibility compatible(Type *t);

  virtual std::string displayName();

  virtual AnyType* clone();

  virtual llvm::Type *typeGen(CodegenContext *cc);
};
