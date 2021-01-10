#include "FCall.h"
#include "../base.h"
#include "../LLContext.h"
#include "FunctionType.h"
#include "Named.h"

void FCall::compile(CC *cc){
  // TODO call args
  // TODO ensure function exists
  // TODO ensure the arguments match the type
}

void FCall::prepeare(CC *cc){
  // TODO call args
}

Type *FCall::type(CC *cc){
  Named *n = cc->lookup(id);
  FunctionType *ft = dynamic_cast<FunctionType *>(n->type);
  // TODO this is an error
  if(!ft) return 0;
  return ft->returnType;
}

void *FCall::exprgen(CC *cc){
  llvm::Function *f = (llvm::Function*)cc->lookup(id)->alloca;
  // TODO args
  return cc->llc->builder->CreateCall(f->getFunctionType(), f);
}
