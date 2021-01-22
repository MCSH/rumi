#pragma once
#include "Function.h"
#include "Statement.h"

class Directive: public Statement{
 public:

  std::string id;
  AST *top;
  Function *f = 0;
  
  Directive(std::string id, AST *top);

  virtual void set(std::string key, void *value) override;
  virtual void add(std::string key, void *value) override;

  virtual void prepare(CC *cc) override;
  virtual void compile(CC *cc) override;
  virtual void codegen(CC *cc) override;
};
