#pragma once
#include "ast.h"

class Type: public AST{
 public:
  virtual void* typegen(CC *cc) = 0;
};
