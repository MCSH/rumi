#pragma once
#include "ast.h"

class Expression;

class Type: public AST{
 public:
  virtual void* typegen(CC *cc) = 0;
  virtual bool hasOp(CC *cc, std::string op, Expression *rhs) = 0;
  virtual void* opgen(CC *cc, Expression *lhs, std::string op, Expression *rhs)=0;
  virtual Type* optyperesolve(CC *cc, std::string op, Expression *rhs) = 0;
};
