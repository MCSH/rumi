#include "VoidType.h"
#include "../../Context.h"

Compatibility VoidType::compatible(Type *t) {
  return Compatibility::OK; // TODO check
}

std::string VoidType::displayName() { return "void"; }

VoidType *VoidType::clone() { return new VoidType(*this); }

llvm::Type *VoidType::typeGen(Context *cc) {
  return llvm::Type::getVoidTy(cc->context);
}
