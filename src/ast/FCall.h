#pragma once
#include "FunctionType.h"
#include "Statement.h"
#include "Expression.h"
#include <vector>

class FCall: public Expression{
 public:
  std::string id;
  std::vector<Expression *>args;

  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;
  virtual void* exprgen(CC *cc) override;
  virtual Type* type(CC *cc) override;
  virtual void *allocagen(CC *cc) override;

private:
  void argError(FunctionType *ft, CC *cc);
};

class FCallStmt: public Statement{
 public:
  FCall fc;

  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;
  virtual void codegen(CC *cc) override;
};
