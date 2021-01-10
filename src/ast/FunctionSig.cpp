#include "FunctionSig.h"
#include "Statement.h"
#include <llvm/ADT/ArrayRef.h>
#include <llvm/IR/BasicBlock.h>
#include "../base.h"
#include "../LLContext.h"
#include "Type.h"
#include "FunctionType.h"
#include "Named.h"
#include "Arg.h"

void FunctionSig::prepare(CC *cc){
  // TODO
}

void FunctionSig::compile(CC *cc){
  FunctionType *type = new FunctionType();
  type->returnType = returnType;

  Named *named = new Named();
  named -> id = id;
  named -> type = type;
  named -> alloca = 0;
  cc->registerNamed(id, named);

  for(Arg *a: args){
    type->args.push_back(a->type);
  }
}

void FunctionSig::codegen(CC *cc){
  // TODO handle varargs
  llvm::Type* rt = returnType?(llvm::Type*)returnType->typegen(cc):
    llvm::Type::getVoidTy(cc->llc->context);

  std::vector<llvm::Type *> types;
  for(Arg *a: args) types.push_back((llvm::Type *)a->type->typegen(cc));

  auto ft = llvm::FunctionType::get(rt, types, false);

  auto f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, this->id, cc->llc->module);

  Named *named = cc->lookup(id);
  named->alloca = f;

}

void FunctionSig::setId(std::string id){
  this->id = id;
}
