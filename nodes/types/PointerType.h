#pragma once
#include "../Type.h"

class PointerType:public Type{
public:
  Type *base;
  PointerType(Type *b): base(b){}

  virtual PointerType *clone();

  virtual Compatibility compatible(Type *t);

  virtual std::string displayName();

  virtual ~PointerType();

  virtual llvm::Type *typeGen(Context *cc);
};
