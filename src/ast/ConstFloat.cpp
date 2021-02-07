#include "ConstFloat.h"
#include "../base.h"
#include "../LLContext.h"
#include "PrimitiveType.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>

void ConstFloat::compile(CC *cc){}
// TODO check the size
void ConstFloat::prepare(CC *cc){}

void *ConstFloat::exprgen(CC *cc){
  if(is32)
    return llvm::ConstantFP::get(llvm::Type::getFloatTy(cc->llc->context), value);
  return llvm::ConstantFP::get(llvm::Type::getDoubleTy(cc->llc->context), value);
}

ConstFloat::ConstFloat(double value, bool is32)
  : value(value)
  , is32(is32)
{}

Type *ConstFloat::type(CC *cc){
  if(is32) return new PrimitiveType(t_f32);
  return new PrimitiveType(t_f64);
}


void *ConstFloat::allocagen(CC *cc){
  graceFulExit(dbg, "Calling alloca on unsupported type");
  return 0;
}
