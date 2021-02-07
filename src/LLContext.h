#pragma once

#include "llvm/IR/IRBuilder.h"

class LLContext{
 public:
  LLContext();
  llvm::LLVMContext context;
  llvm::Module *module;
  llvm::IRBuilder<> *builder;
  llvm::Function *f;

  bool isBrOk(llvm::IRBuilder<> *builder=0); // is it okay to add a br (or return) here?
};
