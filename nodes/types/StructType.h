#pragma once
#include "../Type.h"

class StructType:public Type{
public:
  std::string *name;
  StructType(std::string *name):name(name){}

  virtual Compatibility compatible(Type *t);

  virtual StructType* clone();

  virtual std::string displayName();

  virtual ~StructType();

  virtual llvm::Type *typeGen(Context *cc);
};
