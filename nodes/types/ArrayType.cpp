#include "ArrayType.h"
#include "PointerType.h"
#include <llvm/IR/DerivedTypes.h>


ArrayType *ArrayType::clone(){
    if(count){
      return new ArrayType(base->clone(), count);
    }
    if(exp)
      return new ArrayType(base->clone(), exp);
    return new ArrayType(base->clone());
  }

Compatibility ArrayType::compatible(Type *t){
    auto tid = typeid(*t).hash_code();
    if(tid!= typeid(ArrayType).hash_code() && tid!=typeid(PointerType).hash_code())
      return Compatibility::UNCOMPATIBLE;

    if(tid == typeid(PointerType).hash_code())
      return base->compatible(((PointerType*)t)->base);

    auto at = (ArrayType*) t;

    // TODO check for size, etc

    return base->compatible(at->base);
  }

std::string ArrayType::displayName(){
    if(count)
      return "Array of " + base->displayName() + " of size " + std::to_string(count);
    if(exp)
      return "Array of " + base->displayName() + " of unknown size at runtime";
    return "Array of " + base->displayName();
  }

ArrayType::~ArrayType(){
    delete base;
    if(exp)
      delete exp;
  }

llvm::Type *ArrayType::typeGen(CodegenContext *cc){
  // check for count
  if (this->count)
    return llvm::ArrayType::get(this->base->typeGen(cc), this->count);
  else if (this->exp)
    return this->base->typeGen(cc);
  else
    return llvm::PointerType::getUnqual(this->base->typeGen(cc));
}
