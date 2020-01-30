#include "ArrayExpr.h"
#include "../types/ArrayType.h"
#include "../../Context.h"

ArrayExpr::~ArrayExpr() {
  delete e;
  delete mem;
}
llvm::Value *ArrayExpr::exprGen(CodegenContext *cc) {
  auto member_ptr = this->getAlloca(cc);
  llvm::Value *loaded_member = cc->builder->CreateLoad(member_ptr, "loadtmp");
  return loaded_member;
}

llvm::Value *ArrayExpr::getAlloca(CodegenContext *cc) {
  ArrayType *at = (ArrayType *)this->e->exprType;

  // check for count member
  if (at->count) {
    auto t = this->e->exprType->typeGen(cc);
    auto alloc = this->e->getAlloca(cc);
    // TODO check alloc here and everywhere else!
    auto ind = this->mem->exprGen(cc);

    std::vector<llvm::Value *> indices(2);
    indices[0] = llvm::ConstantInt::get(cc->context, llvm::APInt(64, 0, true));
    indices[1] = ind;
    return cc->builder->CreateInBoundsGEP(t, alloc, indices, "arrptr");
  } else if (at->exp) {
    // This is basically a pointer
    auto t = this->e->exprType->typeGen(cc);
    auto alloc = this->e->getAlloca(cc);
    std::vector<llvm::Value *> indices(1);
    indices[0] = this->mem->exprGen(cc);
    auto tmp = cc->builder->CreateInBoundsGEP(t, alloc, indices, "ptrarrptr");
    return tmp;
  } else {
    // I have no clue how this is working, but it does.
    auto t = this->e->exprType->typeGen(cc);
    auto alloc = this->e->getAlloca(cc);
    alloc = cc->builder->CreateLoad(alloc);
    std::vector<llvm::Value *> indices(1);
    indices[0] = this->mem->exprGen(cc);
    return cc->builder->CreateInBoundsGEP(t->getPointerElementType(), alloc,
                                          indices, "ptrarrptr");
  }
}
void ArrayExpr::compile(CompileContext *cc) {
  resolveType(cc);
  e->compile(cc);
  mem->compile(cc);
  // TODO check to see if ae->e is array
  // TODO check to see if ae->mem is number
}

Type *ArrayExpr::resolveType(CompileContext *cc) {
  if (exprType)
    return exprType;
  auto t = e->resolveType(cc);
  auto bt = (ArrayType *)t;
  exprType = bt->base->clone();
  return exprType;
}
