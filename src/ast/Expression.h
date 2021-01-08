#pragma once
#include "ast.h"

class Expression: public AST{
 public:
  virtual void* exprgen(CC *cc) = 0;
};
