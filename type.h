#pragma once
#include <typeinfo>
#include <cstdlib>
#include <cstdio>
#include <string>

#include <llvm/IR/Type.h>

enum Compatibility { OK = 1, ImpCast = 2, ExpCast = 3, UNCOMPATIBLE = 4 };

class CodegenContext;

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
  virtual llvm::Type *typeGen(CodegenContext *cc)=0;
};

class NoType: public Type{
  virtual Compatibility compatible(Type *t){
    if(typeid(*t).hash_code() == typeid(NoType).hash_code())
      return Compatibility::OK;
    return Compatibility::UNCOMPATIBLE;
  }
  virtual llvm::Type *typeGen(CodegenContext *cc);
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
  virtual llvm::Type *typeGen(CodegenContext *cc);
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
  virtual llvm::Type *typeGen(CodegenContext *cc);
};

class AnyType: public Type{
  virtual Compatibility compatible(Type *t){
    return Compatibility::OK;
  }

  virtual std::string displayName(){
    return "any";
  }

  virtual AnyType* clone(){
    return new AnyType(*this);
  }
  virtual llvm::Type *typeGen(CodegenContext *cc);
};

class VoidType: public Type{
  virtual Compatibility compatible(Type *t){
    return Compatibility::OK; // TODO check
  }

  virtual std::string displayName(){
    return "void";
  }

  virtual VoidType* clone(){
    return new VoidType(*this);
  }
  virtual llvm::Type *typeGen(CodegenContext *cc);
};

class StringType: public Type{
  virtual Compatibility compatible(Type *t){
    if(typeid(*t).hash_code() == typeid(StringType).hash_code())
      return Compatibility::OK;
    return Compatibility::UNCOMPATIBLE;
  }

  virtual std::string displayName(){
    return "string";
  }

  virtual StringType* clone(){
    return new StringType(*this);
  }
  virtual llvm::Type *typeGen(CodegenContext *cc);
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
  virtual llvm::Type *typeGen(CodegenContext *cc);
};

class InterfaceType: public Type{
public:
  std::string *name;
  InterfaceType(std::string *name):name(name){}

  virtual InterfaceType* clone(){
    return new InterfaceType(new std::string(*name));
  }

  virtual Compatibility compatible(Type *t){
    // handled in cast compiler.cpp
    return UNCOMPATIBLE;
  }

  virtual std::string displayName(){
    return "interface " + *name;
  }

  virtual ~InterfaceType(){
    delete name;
  }
  virtual llvm::Type *typeGen(CodegenContext *cc);
};


class PointerType:public Type{
public:
  Type *base;
  PointerType(Type *b): base(b){}

  virtual PointerType *clone(){
    return new PointerType(base->clone());
  }

  virtual Compatibility compatible(Type *t);

  virtual std::string displayName(){
    return "*" + base->displayName();
  }

  virtual ~PointerType(){
    delete base;
  }
  virtual llvm::Type *typeGen(CodegenContext *cc);
};


class Statement;
class TypeNode;

class FunctionType: public Type{
public:
  Type *returnType;
  std::vector<Type*> *args;
  FunctionType(std::vector<Statement *> *args, Type* returnType);

  FunctionType(std::vector<Type *> *args, Type *returnType): returnType(returnType), args(args){}

  virtual FunctionType *clone(){
    // TODO
    std::vector<Type *> *nargs = new std::vector<Type *>();

    for(auto a: *args){
      nargs->push_back(a->clone());
    }

    return new FunctionType(nargs, returnType->clone());
  }


  virtual std::string displayName(){

    std::string argsName = "";
    for(auto arg: *args){
      argsName = argsName + arg->displayName() + ", ";
    }

    return "Function of type ("+argsName+")->"+returnType->displayName();
  }

  virtual Compatibility compatible(Type *t){
    if(typeid(*t).hash_code()!= typeid(FunctionType).hash_code())
      return Compatibility::UNCOMPATIBLE;

    // TODO this could be improved

    auto ft = (FunctionType*) t;

    if(ft->args->size() != args->size())
      return Compatibility::UNCOMPATIBLE;

    Compatibility ans = returnType->compatible(ft->returnType);

    for(int i = 0 ; i < args->size(); i ++){
      Type *tmp = (*args)[i];
      Type *tmp2 = (*ft->args)[i];
      ans = std::max(ans, tmp->compatible(tmp2));
    }
    
    return ans;
  }

  virtual ~FunctionType(){
    delete returnType;
    delete args;
  }
  virtual llvm::Type *typeGen(CodegenContext *cc);
};

class Expression;

class ArrayType: public Type{
public:
  Type *base = 0;
  int count = 0;
  Expression *exp = 0;

  ArrayType(Type *b, Expression *e): base(b), exp(e){}
  ArrayType(Type *b, int count): base(b), count(count){}
  ArrayType(Type *b): base(b){}

  virtual ArrayType *clone(){
    if(count){
      return new ArrayType(base->clone(), count);
    }
    if(exp)
      return new ArrayType(base->clone(), exp);
    return new ArrayType(base->clone());
  }

  virtual Compatibility compatible(Type *t){
    auto tid = typeid(*t).hash_code();
    if(tid!= typeid(ArrayType).hash_code() && tid!=typeid(PointerType).hash_code())
      return Compatibility::UNCOMPATIBLE;

    if(tid == typeid(PointerType).hash_code())
      return base->compatible(((PointerType*)t)->base);

    auto at = (ArrayType*) t;

    // TODO check for size, etc

    return base->compatible(at->base);
  }

  virtual std::string displayName(){
    if(count)
      return "Array of " + base->displayName() + " of size " + std::to_string(count);
    if(exp)
      return "Array of " + base->displayName() + " of unknown size at runtime";
    return "Array of " + base->displayName();
  }

  virtual ~ArrayType(){
    delete base;
    if(exp)
      delete exp;
  }
  virtual llvm::Type *typeGen(CodegenContext *cc);
};
