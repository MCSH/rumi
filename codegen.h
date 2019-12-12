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

class BlockContext{
public:
  std::map<std::string, llvm::AllocaInst *> variables;
  llvm::BasicBlock *bblock;

  BlockContext(llvm::BasicBlock *bb):bblock(bb){}
  BlockContext(){}
};

class CompileContext{
  BlockContext *currentBlock(){
    if(block.size()!=0)
      return block.back();
    return &global;
  }
 public:
  llvm::LLVMContext context;
  std::unique_ptr<llvm::Module> module;
  llvm::IRBuilder<> *builder;
  BlockContext global;
  std::vector<BlockContext *> block;

  llvm::AllocaInst *getVariable(std::string *name);
  void setVariable(std::string *name, llvm::AllocaInst *var);

  CompileContext(){}
};

CompileContext* codegen(std::vector<Statement *> *statements, std::string outfile);
