#pragma once
#include "FCall.h"
#include "MemAccess.h"
#include "Statement.h"
#include "Expression.h"
#include "Method.h"
#include "Struct.h"
#include <vector>
#include "Interface.h"

class MethodCall: public Expression{
 public:
  MemAccess *exp;
  std::vector<Expression *>args;

  Method *method;
  StructType *s;
  Interface *i = 0;
  FunctionSig *fs;
  FCall *fcall;

  virtual void set(std::string key, void *value) override;
  virtual void add(std::string key, void *value) override;

  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;
  virtual void* exprgen(CC *cc) override;
  virtual Type* type(CC *cc) override;
  virtual void *allocagen(CC *cc) override;

private:
  void compileInterface(CC *cc);
  void *exprgenInterface(CC *cc);
};

class MethodCallStmt: public Statement{
 public:
  MethodCall *mc;

  MethodCallStmt(MethodCall *mc);

  virtual void set(std::string key, void *value) override;
  virtual void add(std::string key, void *value) override;

  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;
  virtual void codegen(CC *cc) override;
};
