#include "AnyType.h"
#include "../../Context.h"

Compatibility AnyType::compatible(Type *t) { return Compatibility::OK; }

std::string AnyType::displayName() { return "any"; }

AnyType *AnyType::clone() { return new AnyType(*this); }

llvm::Type *AnyType::typeGen(Context *cc){
  return llvm::Type::getInt64PtrTy(cc->context);
}

