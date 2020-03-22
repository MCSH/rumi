#include "CastExpr.h"
#include "../../Context.h"

CastExpr::~CastExpr() {
  delete t;
  delete exp;
}
llvm::Value *CastExpr::exprGen(CodegenContext *cc) {
  auto targetType = this->exp->exprType;
  auto baseType = this->t;
  return castGen(targetType, baseType, this->exp->exprGen(cc), cc, this, true);
}

llvm::Value *CastExpr::getAlloca(CodegenContext *cc) {}

void CastExpr::compile(Context *cc) {
  resolveType(cc);
  // TODO use cast compile
  exp->compile(cc);
}

Type *CastExpr::resolveType(Context *cc) {
  if (exprType)
    return exprType;
  exp->resolveType(cc); // TODO maybe even healthcheck?
  exprType = t->clone();
  return exprType;
}
