#pragma once
#include "Statement.h"
#include <vector>

class BlockContext;

class CodeBlock: public Statement{
 public:
  std::vector<Statement *> stmts;
  BlockContext *b;

  virtual void set(std::string key, void *value) override;
  virtual void add(std::string key, void *value) override;
  
  virtual void prepare(CC *cc) override;
  virtual void compile(CC *cc) override;
  virtual void codegen(CC *cc) override;
};
