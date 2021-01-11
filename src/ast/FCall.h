#pragma once
#include "Statement.h"
#include "Expression.h"
#include <vector>

class FCall: public Expression{
 public:
  std::string id;
  std::vector<Expression *>args;

  virtual void compile(CC *cc);
  virtual void prepare(CC *cc);
  virtual void* exprgen(CC *cc);
  virtual Type* type(CC *cc);
  virtual void *allocagen(CC *cc);
};

class FCallStmt: public Statement{
 public:
  FCall fc;
  virtual void compile(CC *cc);
  virtual void prepare(CC *cc);
  virtual void codegen(CC *cc);
};
