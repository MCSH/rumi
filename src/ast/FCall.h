#pragma once
#include "Statement.h"
#include "Expression.h"
#include <vector>

class FCall: public Expression{
 public:
  std::string id;
  std::vector<Expression *>args;

  virtual void set(std::string key, void *value) override;
  virtual void add(std::string key, void *value) override;

  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;
  virtual void* exprgen(CC *cc) override;
  virtual Type* type(CC *cc) override;
  virtual void *allocagen(CC *cc) override;
};

class FCallStmt: public Statement{
 public:
  FCall fc;

  virtual void set(std::string key, void *value) override;
  virtual void add(std::string key, void *value) override;

  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;
  virtual void codegen(CC *cc) override;
};
