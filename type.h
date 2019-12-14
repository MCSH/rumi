#pragma once
#include <typeinfo>
#include <cstdlib>
#include <cstdio>

class Type{
public:
  virtual ~Type(){
  }

  virtual bool compatible(Type *t){
    printf("Unimplemented compatible for a type, %s\n", typeid(*this).name());
    exit(1);
  }

  virtual Type* clone() = 0;
};

class NoType: public Type{
  virtual bool compatible(Type *t){
    if(typeid(*t).hash_code() == typeid(NoType).hash_code())
      return true;
    return false;
  }
};

class IntType: public Type{
  virtual bool compatible(Type *t){
    if(typeid(*t).hash_code() == typeid(IntType).hash_code())
      return true;
    return false;
  }

  virtual IntType* clone(){
    return new IntType(*this);
  }
};
