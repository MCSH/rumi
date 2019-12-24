#pragma once
#include <typeinfo>
#include <cstdlib>
#include <cstdio>
#include <string>

enum Compatibility { OK = 1, ImpCast = 2, ExpCast = 3, UNCOMPATIBLE = 4 };

class Type{
public:
  virtual ~Type(){
  }

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
};

class NoType: public Type{
  virtual Compatibility compatible(Type *t){
    if(typeid(*t).hash_code() == typeid(NoType).hash_code())
      return Compatibility::OK;
    return Compatibility::UNCOMPATIBLE;
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
  
  virtual Compatibility compatible(Type *t){
    if(typeid(*t).hash_code() != typeid(IntType).hash_code())
      return Compatibility::UNCOMPATIBLE;

    IntType *target = (IntType*)t;

    if(this->size == 0){
      if(target->size == 0)
        return Compatibility::OK;
      return Compatibility::ImpCast; // TODO
    }

    if(target->size > this->size)
      return Compatibility::ExpCast;

    // TODO float?

    if(target->size == this->size)
      return Compatibility::OK;

    return Compatibility::ImpCast;
  }

  virtual IntType* clone(){
    return new IntType(*this);
  }
};

class FloatType: public Type{
public:
  int size;

  FloatType(int s=0): size(s){
    if(!size){
      // size = 64; // TODO set to system_arch
    }
  }

  virtual std::string displayName(){
    return "Float" + std::to_string(size);
  }
  
  virtual Compatibility compatible(Type *t){
    if(typeid(*t).hash_code() != typeid(FloatType).hash_code()){
      if(typeid(*t).hash_code() != typeid(IntType).hash_code())
        return Compatibility::UNCOMPATIBLE;
      return ((IntType*)t)->size <= size
        ? Compatibility::ImpCast
        : Compatibility::ExpCast;
    }

    FloatType *target = (FloatType*)t;

    if(target->size > this->size)
      return Compatibility::UNCOMPATIBLE;

    if(target->size == this->size)
      return Compatibility::OK;

    return Compatibility::ImpCast;
  }

  virtual FloatType* clone(){
    return new FloatType(*this);
  }
};

class AnyType: public Type{
  virtual Compatibility compatible(Type *t){
    return Compatibility::OK;
  }

  virtual AnyType* clone(){
    return new AnyType(*this);
  }
};

class VoidType: public Type{
  virtual Compatibility compatible(Type *t){
    return Compatibility::OK; // TODO check
  }

  virtual VoidType* clone(){
    return new VoidType(*this);
  }
};

class StringType: public Type{
  virtual Compatibility compatible(Type *t){
    if(typeid(*t).hash_code() == typeid(StringType).hash_code())
      return Compatibility::OK;
    return Compatibility::UNCOMPATIBLE;
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

  virtual Compatibility compatible(Type *t){
    if(typeid(*t).hash_code() != typeid(StructType).hash_code())
      return Compatibility::UNCOMPATIBLE;
    // TODO polymorphism
    if(name->compare(*((StructType*)t)->name)==0)
      return Compatibility::OK;
    return Compatibility::UNCOMPATIBLE;
  }

  virtual StructType* clone(){
    return new StructType(new std::string(*name));
  }

  virtual std::string displayName(){
    return "struct " + *name;
  }

  virtual ~StructType(){
    delete name;
  }
};


class PointerType:public Type{
public:
  Type *base;
  PointerType(Type *b): base(b){}

  virtual PointerType *clone(){
    return new PointerType(base->clone());
  }

  virtual Compatibility compatible(Type *t){
    if(typeid(*t).hash_code() != typeid(PointerType).hash_code()){
      return Compatibility::UNCOMPATIBLE; // TODO exceptions such as arrays and strings
    }

    PointerType *pt = (PointerType*)t;

    return base->compatible(pt->base);
  }

  virtual std::string displayName(){
    return "*" + base->displayName();
  }

  virtual ~PointerType(){
    delete base;
  }
};
  
// ArrayType moved to Node.h
