#include "ConstBool.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include "PrimitiveType.h"
#include "../base.h"
#include "../LLContext.h"
#include "ast.h"

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
  graceFulExit(dbg, "Calling alloca on unsupported type");
  return 0;
}

