#pragma once
#include "ast.h"

class Statement: public AST{
 public:
  void codegen(CC *cc);
};
