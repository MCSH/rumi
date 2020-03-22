#include "CastExpr.h"
#include "../../Context.h"
#include "../../codegen.h"

CastExpr::~CastExpr() {
  delete t;
  delete exp;
}
llvm::Value *CastExpr::exprGen(Context *cc) {
  auto targetType = this->exp->exprType;
  auto baseType = this->t;
  return castGen(targetType, baseType, this->exp->exprGen(cc), cc, this, true);
}

llvm::Value *CastExpr::getAlloca(Context *cc) {}

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
