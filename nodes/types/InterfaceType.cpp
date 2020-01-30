#include "InterfaceType.h"
#include "../../Context.h"

InterfaceType *InterfaceType::clone() {
  return new InterfaceType(new std::string(*name));
}

Compatibility InterfaceType::compatible(Type *t) {
  // handled in cast compiler.cpp
  return UNCOMPATIBLE;
}

std::string InterfaceType::displayName() { return "interface " + *name; }

InterfaceType::~InterfaceType() { delete name; }

llvm::Type *InterfaceType::typeGen(CodegenContext *cc){
  return cc->getInterfaceType(name);
}
