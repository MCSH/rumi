#include "ConstBool.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include "PrimitiveType.h"
#include "../base.h"
#include "../LLContext.h"

void ConstBool::compile(CC *cc){}
void ConstBool::prepare(CC *cc){}

void *ConstBool::exprgen(CC *cc){
  return llvm::ConstantInt::get(llvm::IntegerType::getInt1Ty(cc->llc->context), truth?1:0, false);
}

ConstBool::ConstBool(bool truth)
  : truth(truth)
{}

Type *ConstBool::type(CC *cc){
  return new PrimitiveType(t_bool);
}


void *ConstBool::allocagen(CC *cc){
  cc->debug(NONE) << "Calling alloca on unsupported type" << std::endl;
  exit(1);
}
