#include "IntType.h"
#include "../../Context.h"

llvm::Type *IntType::typeGen(CodegenContext *cc){
  switch(this->size){
  case 0:
    /// TODO do it based on sys arch
    return llvm::Type::getInt64Ty(cc->context);
  case 1:
    return llvm::Type::getInt1Ty(cc->context);
  case 8:
    return llvm::Type::getInt8Ty(cc->context);
  case 16:
    return llvm::Type::getInt16Ty(cc->context);
  case 32:
    return llvm::Type::getInt32Ty(cc->context);
  case 64:
    return llvm::Type::getInt64Ty(cc->context);
  }

  printf("Something went wrong on intTypeGen\n");
  exit(1);
  return nullptr;
}

std::string IntType::displayName() {
  std::string s = isSigned ? "signed " : "unSigned ";
  return s + "Int" + std::to_string(size);
}

Compatibility IntType::compatible(Type *t){
  if (typeid(*t).hash_code() != typeid(IntType).hash_code())
    return Compatibility::UNCOMPATIBLE;

  IntType *target = (IntType *)t;

  if (this->size == 0) {
    if (target->size == 0)
      return Compatibility::OK;
    return Compatibility::ImpCast; // TODO
  }

  if (target->size > this->size)
    return Compatibility::ExpCast;

  // TODO float?

  if (target->size == this->size)
    return Compatibility::OK;

  return Compatibility::ImpCast;
}

IntType *IntType::clone() { return new IntType(*this); }
