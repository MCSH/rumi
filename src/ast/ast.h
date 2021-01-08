#pragma once
#include "../CC.h"
#include <set>

class AST{
 public:

  std::set<AST *> compileDeps;
  std::set<AST *> linkDeps;
  
  virtual void compile(CC *cc) = 0; // identify the link dependency
  virtual void prepeare(CC *cc) = 0; // identify the compile dependency

  virtual ~AST() = 0;
};
