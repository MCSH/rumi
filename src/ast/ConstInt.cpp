#include "ConstInt.h"
#include "../base.h"
#include "../LLContext.h"
#include "PrimitiveType.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>

void ConstInt::compile(CC *cc){}
// TODO check the size
void ConstInt::prepare(CC *cc){}

void *ConstInt::exprgen(CC *cc){
  return llvm::ConstantInt::get(llvm::IntegerType::getInt64Ty(cc->llc->context), value, true);
}

ConstInt::ConstInt(long long value)
  : value(value)
{}

Type *ConstInt::type(CC *cc){
  return new PrimitiveType(t_int);
}
