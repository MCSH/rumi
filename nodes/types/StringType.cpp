#include "StringType.h"
#include "../../Context.h"

#include <llvm/IR/DerivedTypes.h>


Compatibility StringType::compatible(Type *t){
    if(typeid(*t).hash_code() == typeid(StringType).hash_code())
      return Compatibility::OK;
    return Compatibility::UNCOMPATIBLE;
  }

std::string StringType::displayName(){
    return "string";
  }

StringType* StringType::clone(){
    return new StringType(*this);
  }

llvm::Type *StringType::typeGen(CodegenContext *cc){
  // Improve?
  return llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(cc->context));
}
