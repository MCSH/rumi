#pragma once
#include "../Type.h"

class FloatType: public Type{
public:
  int size;

  FloatType(int s=0): size(s){}

  virtual std::string displayName();
  
  virtual Compatibility compatible(Type *t);

  virtual FloatType* clone();

  virtual llvm::Type *typeGen(Context *cc);
};
