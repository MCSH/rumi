#pragma once
#include "Statement.h"
#include <vector>

class PackedAST: public Statement{
 public:
  std::vector<AST *> stmts;

  virtual void prepare(CC *cc) override;
  virtual void compile(CC *cc) override;
  virtual void codegen(CC *cc) override;
};
