#include "ConstInt.h"
#include "../base.h"
#include "../LLContext.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>

void ConstInt::compile(CC *cc){}
void ConstInt::prepeare(CC *cc){}

void *ConstInt::exprgen(CC *cc){
  return llvm::ConstantInt::get(llvm::IntegerType::getInt64Ty(cc->llc->context), value, true);
}

ConstInt::ConstInt(long long value)
  : value(value)
{}
