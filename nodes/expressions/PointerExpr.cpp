#include "PointerExpr.h"
#include "../types/PointerType.h"

PointerExpr::~PointerExpr() { delete exp; }
llvm::Value *PointerExpr::exprGen(Context *cc) {
  return exp->getAlloca(cc);
}
llvm::Value *PointerExpr::getAlloca(Context *cc) {}

void PointerExpr::compile(Context *cc) {
  resolveType(cc);
  // TODO
}

Type *PointerExpr::resolveType(Context *cc) {
  if (exprType)
    return exprType;
  exprType = new PointerType(exp->resolveType(cc)->clone());
  return exprType;
}
