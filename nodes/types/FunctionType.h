#pragma once
#include "../Type.h"
#include <vector>

class Statement;
class TypeNode;

class FunctionType: public Type{
public:
  Type *returnType;
  std::vector<Type*> *args;
  FunctionType(std::vector<Statement *> *args, Type* returnType);

  FunctionType(std::vector<Type *> *args, Type *returnType): returnType(returnType), args(args){}

  virtual FunctionType *clone();

  virtual std::string displayName();

  virtual Compatibility compatible(Type *t);

  virtual ~FunctionType();

  virtual llvm::Type *typeGen(Context *cc);
};
