#pragma once
#include "Statement.h"
#include "Function.h"

class Method: public Statement{
 public:
  std::string structName, methodName;
  Function *f;

  Method(std::string structName, std::string methodName, Function *f);
  
  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;
  virtual void codegen(CC *cc) override;
};
