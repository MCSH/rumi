#pragma once
#include <typeinfo>
#include <cstdlib>
#include <cstdio>
#include <string>

class Type{
public:
  virtual ~Type(){
  }

  virtual bool compatible(Type *t){
    printf("Unimplemented compatible for a type, %s\n", typeid(*this).name());
    exit(1);
  }

  virtual std::string displayName(){
    printf("Unimplemented displayName for a type, %s\n", typeid(*this).name());
    exit(1);
    return 0;
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
public:
  int size;
  bool isSigned;

  IntType(int s=0, bool si=1): size(s), isSigned(si){
    if(!size){
      // size = 64; // TODO set to system_arch
    }
  }

  virtual std::string displayName(){
    std::string s = isSigned? "signed ": "unSigned ";
    return s + "Int" + std::to_string(size);
  }
  
  virtual bool compatible(Type *t){
    if(typeid(*t).hash_code() != typeid(IntType).hash_code())
      return false;

    IntType *target = (IntType*)t;

    if(target->size > this->size)
      return false;

    return true;
  }

  virtual IntType* clone(){
    return new IntType(*this);
  }
};

class AnyType: public Type{
  virtual bool compatible(Type *t){
    return true;
  }

  virtual AnyType* clone(){
    return new AnyType(*this);
  }
};

class StringType: public Type{
  virtual bool compatible(Type *t){
    if(typeid(*t).hash_code() == typeid(StringType).hash_code())
      return true;
    return false;
  }

  virtual StringType* clone(){
    return new StringType(*this);
  }
};

class StructType:public Type{
public:
  std::string *name;
  StructType(std::string *name):name(name){

  }

  virtual StructType* clone(){
    return new StructType(new std::string(*name));
  }

  virtual ~StructType(){
    delete name;
  }
};
