#pragma once

#include "llvm/IR/IRBuilder.h"

class LLContext{
 public:
  LLContext();
  llvm::LLVMContext context;
  llvm::Module *module;
  llvm::IRBuilder<> *builder;
};
