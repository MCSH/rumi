#pragma once
#include "../Expression.h"
#include <string>

class MemberExpr: public Expression{
public:
  Expression *e;
  std::string *mem;
  int level=0;
  MemberExpr(Expression *e, std::string *mem): mem(mem), e(e){}

  virtual ~MemberExpr();
  virtual llvm::Value *exprGen(CodegenContext *cc);
  virtual llvm::Value *getAlloca(CodegenContext *cc);
  virtual void compile(Context *cc);
  virtual Type *resolveType(Context *cc);
};
