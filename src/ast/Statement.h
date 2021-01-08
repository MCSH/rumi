#pragma once
#include "ast.h"

class Statement: public AST{
 public:
  virtual void codegen(CC *cc) = 0;
};
