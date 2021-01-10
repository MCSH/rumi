#include "FCall.h"
#include "../base.h"
#include "../LLContext.h"
#include "FunctionType.h"
#include "Named.h"

void FCall::compile(CC *cc){
  // TODO ensure function exists
  // TODO ensure the arguments match the type
  for(auto e: args) e->compile(cc);
}

void FCall::prepare(CC *cc){
  for(auto e: args) e->prepare(cc);
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
  std::vector<llvm::Value *> params;
  for(auto e: args){
    params.push_back((llvm::Value *)e->exprgen(cc));
  }
  return cc->llc->builder->CreateCall(f->getFunctionType(), f, params);
}
