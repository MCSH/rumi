#pragma once
#include "node.h"

#include "llvm/Bitcode/BitcodeWriter.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

// #include "llvm/Passes/PassBuilder.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/MemoryBuffer.h"

class CompileContext{
 public:
  llvm::LLVMContext context;
  std::unique_ptr<llvm::Module> module;
  llvm::IRBuilder<> *builder;
};

CompileContext* codegen(std::vector<Statement *> *statements, std::string outfile);
