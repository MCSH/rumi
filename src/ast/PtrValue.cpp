#include "PtrValue.h"
#include "../base.h"
#include "../LLContext.h"
#include "PointerType.h"

PtrValue::PtrValue(Expression *ptr)
  : ptr(ptr)
{}

void PtrValue::compile(CC *cc){
  ptr->compile(cc);

  // type check
  PointerType *pt = dynamic_cast<PointerType *>(ptr->type(cc));
  if(!pt){
    cc->debug(NONE) << "Accessign pointer value of a non-pointer expression" << std::endl;
    exit(1);
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
