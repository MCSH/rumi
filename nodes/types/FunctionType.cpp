#include "FunctionType.h"
#include "../expressions/TypeNode.h"
#include <llvm/IR/DerivedTypes.h>

FunctionType::FunctionType(std::vector<Statement *> *args, Type *returnType)
    : returnType(returnType) {
  this->args = new std::vector<Type *>();
  for (auto a : *args) {
    TypeNode *tn = (TypeNode *)a;
    this->args->push_back(tn->exprType->clone());
    delete tn;
  }
  delete args;
}

FunctionType *FunctionType::clone() {
  // TODO
  std::vector<Type *> *nargs = new std::vector<Type *>();

  for (auto a : *args) {
    nargs->push_back(a->clone());
  }

  return new FunctionType(nargs, returnType->clone());
}

std::string FunctionType::displayName() {

  std::string argsName = "";
  for (auto arg : *args) {
    argsName = argsName + arg->displayName() + ", ";
  }

  return "Function of type (" + argsName + ")->" + returnType->displayName();
}

Compatibility FunctionType::compatible(Type *t) {
  if (typeid(*t).hash_code() != typeid(FunctionType).hash_code())
    return Compatibility::UNCOMPATIBLE;

  // TODO this could be improved

  auto ft = (FunctionType *)t;

  if (ft->args->size() != args->size())
    return Compatibility::UNCOMPATIBLE;

  Compatibility ans = returnType->compatible(ft->returnType);

  for (int i = 0; i < args->size(); i++) {
    Type *tmp = (*args)[i];
    Type *tmp2 = (*ft->args)[i];
    ans = std::max(ans, tmp->compatible(tmp2));
  }

  return ans;
}

FunctionType::~FunctionType() {
  delete returnType;
  delete args;
}

llvm::Type *FunctionType::typeGen(CodegenContext *cc){
  std::vector<llvm::Type *> params;

    for(auto arg: *this->args)
      params.push_back(arg->typeGen(cc));

    // TODO this should be a poitner by default

    return llvm::PointerType::getUnqual(llvm::FunctionType::get(
        this->returnType->typeGen(cc), params, false)); // TODO varargs
}
