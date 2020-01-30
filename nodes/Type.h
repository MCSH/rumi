#pragma once
#include <cstdio>
#include <cstdlib>
#include <string>
#include <typeinfo>

#include <llvm/IR/Type.h>

class CodegenContext;

enum Compatibility { OK = 1, ImpCast = 2, ExpCast = 3, UNCOMPATIBLE = 4 };

class Type{
public:
  virtual ~Type(){
  }

  // TODO maybe include context for things like struct and interface?
  virtual Compatibility compatible(Type *t){
    printf("Unimplemented compatible for a type, %s\n", typeid(*this).name());
    exit(1);
  }

  virtual std::string displayName(){
    printf("Unimplemented displayName for a type, %s\n", typeid(*this).name());
    exit(1);
    return 0;
  }

  virtual Type* clone() = 0;
  virtual llvm::Type *typeGen(CodegenContext *cc)=0;
};
