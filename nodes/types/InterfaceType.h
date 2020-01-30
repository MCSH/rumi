#pragma once
#include "../Type.h"

class InterfaceType: public Type{
public:
  std::string *name;

  InterfaceType(std::string *name):name(name){}

  virtual InterfaceType* clone();

  virtual Compatibility compatible(Type *t);

  virtual std::string displayName();

  virtual ~InterfaceType();

  virtual llvm::Type *typeGen(CodegenContext *cc);
};
