#pragma once
#include "ast.h"
#include "Type.h"

class Expression: public AST{
 public:
  virtual void* exprgen(CC *cc) = 0;
  virtual Type* type(CC *cc) = 0;
};
