#include "PointerExpr.h"
#include "../types/PointerType.h"

PointerExpr::~PointerExpr() { delete exp; }
llvm::Value *PointerExpr::exprGen(CodegenContext *cc) {
  return exp->getAlloca(cc);
}
llvm::Value *PointerExpr::getAlloca(CodegenContext *cc) {}

void PointerExpr::compile(CompileContext *cc) {
  resolveType(cc);
  // TODO
}

Type *PointerExpr::resolveType(CompileContext *cc) {
  if (exprType)
    return exprType;
  exprType = new PointerType(exp->resolveType(cc)->clone());
  return exprType;
}
