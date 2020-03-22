#include "FloatType.h"
#include "IntType.h"
#include "../../Context.h"

std::string FloatType::displayName() { return "Float" + std::to_string(size); }

Compatibility FloatType::compatible(Type *t) {
  if (typeid(*t).hash_code() != typeid(FloatType).hash_code()) {
    if (typeid(*t).hash_code() != typeid(IntType).hash_code())
      return Compatibility::UNCOMPATIBLE;
    return ((IntType *)t)->size <= size ? Compatibility::ImpCast
                                        : Compatibility::ExpCast;
  }

  FloatType *target = (FloatType *)t;

  if (target->size > this->size)
    return Compatibility::UNCOMPATIBLE;

  if (target->size == this->size)
    return Compatibility::OK;

  return Compatibility::ImpCast;
}

FloatType *FloatType::clone() { return new FloatType(*this); }

llvm::Type *FloatType::typeGen(Context *cc){
  switch(size){
  case 0:
    // TODO do it based on sys arch
    return llvm::Type::getFloatTy(cc->context);
  case 32:
    return llvm::Type::getFloatTy(cc->context);
  case 64:
    return llvm::Type::getDoubleTy(cc->context);
  }

  printf("Something went wrong on floatTypeGen\n");
  exit(1);
  return nullptr;
}
