#pragma once
#include "../CC.h"
#include <set>

class AST{
 public:
  virtual void compile(CC *cc) = 0; // identify the link dependency
  virtual void prepeare(CC *cc) = 0; // identify the compile dependency

  virtual ~AST() = 0;
};
