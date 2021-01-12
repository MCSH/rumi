#pragma once
#include "ast.h"

class Expression;

enum Compatibility { OK = 1, ImpCast = 2, ExpCast = 3, INCOMPATIBLE = 4 };

class Type: public AST{
 public:
  virtual void* typegen(CC *cc) = 0;
  virtual bool hasOp(CC *cc, std::string op, Expression *rhs) = 0;
  virtual void* opgen(CC *cc, Expression *lhs, std::string op, Expression *rhs)=0;
  virtual Type* optyperesolve(CC *cc, std::string op, Expression *rhs) = 0;

  virtual Compatibility compatible(Type *t) = 0;
  virtual void* castgen(CC *cc, Expression *e) = 0;

  virtual bool hasMem(CC *cc, Expression *exp, std::string id) = 0;
  virtual void* memgen(CC *cc, Expression *exp, std::string id) = 0;
  virtual Type* memtyperesolve(CC *cc, Expression *exp, std::string id) = 0;
  virtual void* memalloca(CC *cc, Expression *exp, std::string id) = 0;
};
