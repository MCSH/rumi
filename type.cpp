#include "type.h"
#include "node.h"

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
