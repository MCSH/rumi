#include "PointerAccessExpr.h"
#include "../../Context.h"

PointerAccessExpr::~PointerAccessExpr() { delete exp; }

llvm::Value *PointerAccessExpr::exprGen(Context *cc) {
  return cc->builder->CreateLoad(exp->exprGen(cc), "ptra");
}

llvm::Value *PointerAccessExpr::getAlloca(Context *cc) {
  auto load = this->exp->exprGen(cc);
  return load;
}

void PointerAccessExpr::compile(Context *cc) {
  resolveType(cc);
  exp->compile(cc);
}

Type *PointerAccessExpr::resolveType(Context *cc) {
  if (exprType)
    return exprType;
  PointerType *pt = (PointerType *)exp->resolveType(cc);
  exprType = pt->base->clone();
  return exprType;
}
