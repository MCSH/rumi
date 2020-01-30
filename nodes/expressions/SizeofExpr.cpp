#include "SizeofExpr.h"
#include "../../Context.h"
#include "../types/IntType.h"
#include "IntValue.h"

SizeofExpr::~SizeofExpr() { delete t; }
llvm::Value *SizeofExpr::exprGen(CodegenContext *cc) {
  auto dl = cc->module->getDataLayout();
  auto size = dl.getTypeAllocSize(this->t->typeGen(cc));
  // TODO memory leak
  return (new IntValue(size))->exprGen(cc);
}

llvm::Value *SizeofExpr::getAlloca(CodegenContext *cc) {}

void SizeofExpr::compile(CompileContext *cc) {
  resolveType(cc);
  // TODO nothing?
}

Type *SizeofExpr::resolveType(CompileContext *cc) {
  if (exprType)
    return exprType;
  exprType = new IntType();
  return exprType;
}
