#include "PtrAssign.h"
#include "../base.h"
#include "../LLContext.h"

PtrAssign::PtrAssign(Expression *ptr, Expression *value)
  : ptr(ptr)
  , value(value)
{}

void PtrAssign::prepare(CC *cc){
  ptr->prepare(cc);
  value->prepare(cc);

  // TODO check types
}

void PtrAssign::compile(CC *cc){
  ptr->compile(cc);
  value->compile(cc);
}

void PtrAssign::codegen(CC *cc){
  auto p = (llvm::Value *)ptr->exprgen(cc);
  auto v = (llvm::Value *)value->exprgen(cc);

  cc->llc->builder->CreateStore(v, p);
}
