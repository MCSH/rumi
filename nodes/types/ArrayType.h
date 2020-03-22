#pragma once
#include "../Type.h"

class Expression;

class ArrayType: public Type{
public:
  Type *base = 0;
  int count = 0;
  Expression *exp = 0;

  ArrayType(Type *b, Expression *e): base(b), exp(e){}
  ArrayType(Type *b, int count): base(b), count(count){}
  ArrayType(Type *b): base(b){}

  virtual ArrayType *clone();

  virtual Compatibility compatible(Type *t);

  virtual std::string displayName();

  virtual ~ArrayType();

  virtual llvm::Type *typeGen(Context *cc);
};
