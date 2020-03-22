#include "PointerType.h"
#include "ArrayType.h"
#include "AnyType.h"
#include "../../Context.h"
#include "StructType.h"

PointerType *PointerType::clone() { return new PointerType(base->clone()); }

Compatibility PointerType::compatible(Type *t) {
  auto tid = typeid(*t).hash_code();
  if (tid != typeid(PointerType).hash_code() &&
      tid != typeid(ArrayType).hash_code()) {
    return Compatibility::UNCOMPATIBLE;
  }

  if (tid == typeid(ArrayType).hash_code()) {
    return base->compatible(((ArrayType*) t)->base);
  }

  PointerType *pt = (PointerType *)t;

  if(typeid(*pt->base).hash_code() == typeid(AnyType).hash_code())
    return Compatibility::ImpCast;

  return base->compatible(pt->base);
}

std::string PointerType::displayName() { return "*" + base->displayName(); }

PointerType::~PointerType(){ delete base; }

llvm::Type *PointerType::typeGen(Context *cc) {
  auto baseType = this->base->typeGen(cc);
  if (!baseType) {
    // Type is not generated yet, it should be a struct I assume

    StructType *st = (StructType *)this->base;
    baseType = cc->module->getTypeByName(*st->name);
  }

  return llvm::PointerType::getUnqual(baseType);
}
