#pragma once
#include "Statement.h"
#include "Function.h"

// TODO expose in compiler API
class BinOpDef: public Statement{
 public:
  Function *f;
  std::string structName, methodName;
  Type *rhs;
  bool firstPtr, secondPtr;
  
  BinOpDef(std::string id, std::string op, Type *rhs, Function *f);

  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;
  virtual void codegen(CC *cc) override;
};
