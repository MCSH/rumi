#pragma once
#include "../Expression.h"
#include "../expressions/FunctionCallExpr.h"
#include "../statements/FunctionDefine.h"
#include <string>
#include <vector>

class MethodCall: public Expression{
public:
  Expression *e;
  std::string *name;
  std::vector<Expression *> *expr;

  FunctionCallExpr *fce;
  FunctionDefine *f;

  int methodInd; // used for interfaces

  MethodCall(Expression *e, std::string *n, std::vector<Expression *> *expr);
 
  virtual ~MethodCall();
  virtual llvm::Value *exprGen(CodegenContext *cc);
  virtual llvm::Value *getAlloca(CodegenContext *cc);
  void codegen(CodegenContext *cc);
  virtual void compile(CompileContext *cc);
  virtual Type *resolveType(CompileContext *cc);
};
