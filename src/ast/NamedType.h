#pragma once
#include "Define.h"
#include "Type.h"
#include <vector>

class NamedType: public Type{
public:
  std::string id;

  Type *resolvedType = 0;

  NamedType(std::string id);

  virtual void* typegen(CC *cc);
  virtual void compile(CC *cc);
  virtual void prepare(CC *cc);
  bool hasOp(CC *cc, std::string op, Expression *rhs);
  void* opgen(CC *cc, Expression *lhs, std::string op, Expression *rhs);
  Type* optyperesolve(CC *cc, std::string op, Expression *rhs);
  Compatibility compatible(Type *t);
  void* castgen(CC *cc, Expression *e);
};
