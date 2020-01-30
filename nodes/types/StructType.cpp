#include "StructType.h"
#include "../../Context.h"

Compatibility StructType::compatible(Type *t) {
  if (typeid(*t).hash_code() != typeid(StructType).hash_code())
    return Compatibility::UNCOMPATIBLE;
  // TODO polymorphism
  if (name->compare(*((StructType *)t)->name) == 0)
    return Compatibility::OK;
  return Compatibility::UNCOMPATIBLE;
}

StructType *StructType::clone() {
  return new StructType(new std::string(*name));
}

std::string StructType::displayName() { return "struct " + *name; }

StructType::~StructType() { delete name; }

llvm::Type *StructType::typeGen(CodegenContext *cc){
  return cc->getStructType(this->name);
}

