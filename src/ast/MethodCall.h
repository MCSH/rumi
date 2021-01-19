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

  virtual void compile(CC *cc);
  virtual void prepare(CC *cc);
  virtual void* exprgen(CC *cc);
  virtual Type* type(CC *cc);
  virtual void *allocagen(CC *cc);

private:
  void compileInterface(CC *cc);
  void *exprgenInterface(CC *cc);
};

class MethodCallStmt: public Statement{
 public:
  MethodCall *mc;

  MethodCallStmt(MethodCall *mc);

  virtual void compile(CC *cc);
  virtual void prepare(CC *cc);
  virtual void codegen(CC *cc);
};
