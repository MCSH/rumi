#pragma once
#include "../Expression.h"
#include <string>
#include <vector>

class FunctionCallExpr: public Expression{
public:
  std::string *name;
  std::vector<Expression *> *expr;

  FunctionCallExpr(std::string *n, std::vector<Expression *> *expr);
  virtual ~FunctionCallExpr();
  virtual llvm::Value *exprGen(CodegenContext *cc);
  virtual llvm::Value *getAlloca(CodegenContext *cc);
  void codegen(CodegenContext *cc);
  virtual void compile(Context *cc);
  virtual Type *resolveType(Context *cc);
};
