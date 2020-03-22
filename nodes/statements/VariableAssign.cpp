#include "VariableAssign.h"
#include "../expressions/VariableExpr.h"
#include "../types/ArrayType.h"
#include "../../Context.h"
#include <llvm/IR/Value.h>

VariableAssign::~VariableAssign() {
  delete base;
  delete exp;
}

void VariableAssign::codegen(CodegenContext *cc) {
  llvm::AllocaInst *alloc = (llvm::AllocaInst *)base->getAlloca(cc);

  if (!alloc) {
    if (typeid(*base).hash_code() == typeid(VariableExpr).hash_code()) {
      printf("Unknown variable %s\nThis is a compiler bug reported from "
             "codegen::variableAssignGen",
             ((VariableExpr *)base)->name->c_str());
    } else {
      printf("I don't know what is happening, we are on variableAssigngen\n");
    }
    exit(1);
  }

  if (typeid(*exp->exprType).hash_code() == typeid(ArrayType).hash_code()) {
    // TODO check?
    auto art = (ArrayType *)exp->exprType;
    // auto arrayAlloc = cc->builder->CreateLoad(getAlloca(va->exp, cc));
    auto arrayAlloc = exp->getAlloca(cc);
    // auto at = typeGen(art->base, cc);
    auto at = art->typeGen(cc);
    auto ptr = arrayToPointer(at, arrayAlloc, cc);
    cc->builder->CreateStore(ptr, alloc);
    return;
  }

  auto e = exp->exprGen(cc);

  // Type check
  // TODO this shouldn't be here, we are checking it in compiler
  auto baseType = base->exprType;
  auto exprType = exp->exprType;
  e = castGen(exprType, baseType, e, cc, this, false, alloc);
  if (e)
    cc->builder->CreateStore(e, alloc);
}

void VariableAssign::compile(Context *cc) {
  auto va = this;
  va->base->compile(cc);
  va->exp->compile(cc);

  // TODO move the cast to here.
  auto baseType = va->base->resolveType(cc);
  auto exprType = va->exp->resolveType(cc);
  va->exp = castCompile(exprType, baseType, va->exp, cc, va, false);
  // TODO
  return;
}
