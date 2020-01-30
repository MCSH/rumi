#pragma once
#include "../Type.h"

class IntType: public Type{
public:
  int size;
  bool isSigned;

  IntType(int s=0, bool si=1): size(s), isSigned(si){}

  virtual std::string displayName();
  
  virtual Compatibility compatible(Type *t);

  virtual IntType* clone();

  virtual llvm::Type *typeGen(CodegenContext *cc);
};
