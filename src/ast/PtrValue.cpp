#include "PtrValue.h"
#include "../base.h"
#include "../LLContext.h"
#include "PointerType.h"

// NOTE: PtrValue should not rely on prepare / compile directly. The inner exp will always get their prepare/compile called, but address might not. (EX: Struct op gen)
PtrValue::PtrValue(Expression *ptr)
  : ptr(ptr)
{}

void PtrValue::compile(CC *cc){
  ptr->compile(cc);

  // type check
  PointerType *pt = dynamic_cast<PointerType *>(ptr->type(cc));
  if(!pt){
    graceFulExit(dbg, "Accessing pointer value of a non-pointer expression");
  }
}

void PtrValue::prepare(CC *cc){
  ptr->prepare(cc);
}

void *PtrValue::exprgen(CC *cc){
  auto p = (llvm::Value*) ptr->exprgen(cc);
  return cc->llc->builder->CreateLoad(p);
}

Type *PtrValue::type(CC *cc){
  PointerType *pt = (PointerType *) ptr->type(cc);
  return pt->innerType;
}

void *PtrValue::allocagen(CC *cc){
  return ptr->exprgen(cc);
}
