#include "PointerAccessExpr.h"
#include "../../Context.h"

PointerAccessExpr::~PointerAccessExpr() { delete exp; }

llvm::Value *PointerAccessExpr::exprGen(CodegenContext *cc) {
  return cc->builder->CreateLoad(exp->exprGen(cc), "ptra");
}

llvm::Value *PointerAccessExpr::getAlloca(CodegenContext *cc) {
  auto load = this->exp->exprGen(cc);
  return load;
}

void PointerAccessExpr::compile(CompileContext *cc) {
  resolveType(cc);
  exp->compile(cc);
}

Type *PointerAccessExpr::resolveType(CompileContext *cc) {
  if (exprType)
    return exprType;
  PointerType *pt = (PointerType *)exp->resolveType(cc);
  exprType = pt->base->clone();
  return exprType;
}
