#include "Sizeof.h"
#include "../base.h"
#include "../LLContext.h"
#include "PrimitiveType.h"

Sizeof::Sizeof(Type *type_)
  : type_(type_)
{}

void Sizeof::compile(CC *cc){
  this->type_->compile(cc);
}

void Sizeof::prepare(CC *cc){
  this->type_->prepare(cc);
}

void *Sizeof::exprgen(CC *cc){
  auto dl = cc->llc->module->getDataLayout();
  auto size = dl.getTypeAllocSize((llvm::Type *)type_->typegen(cc));

  return llvm::ConstantInt::get(llvm::IntegerType::getInt8Ty(cc->llc->context), size, true);
}

Type *Sizeof::type(CC *cc){
  return new PrimitiveType(t_u8);
}

void *Sizeof::allocagen(CC *cc){
  cc->debug(NONE) << "Calling alloca on unsupported type" << std::endl;
  exit(1);
}

void Sizeof::set(std::string key, void *value){
  if(key == "type"){
    type_ = (Type *) value;
    return;
  }

  // TODO error?
}

void Sizeof::add(std::string key, void *value){
  // TODO error?
}
