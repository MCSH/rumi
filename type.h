#pragma once
#include <typeinfo>
#include <cstdlib>
#include <cstdio>

class Type{
public:
  virtual bool compatible(Type *t){
    printf("Unimplemented compatible for a type, %s\n", typeid(*this).name());
    exit(1);
  }
};

class NoType: public Type{
};

class IntType: public Type{
};
