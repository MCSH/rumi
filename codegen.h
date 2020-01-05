#pragma once
#include "node.h"
#include <llvm/IR/Instructions.h>
#include <tuple> 

#include "llvm/Bitcode/BitcodeWriter.h"

#include "llvm/IR/Constants.h"
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


class CodegenBlockContext{
public:
  std::map<std::string, std::tuple<llvm::AllocaInst *, VariableDecl *>*> variables;
  std::map<std::string, std::tuple<llvm::Type *, StructStatement *>*> structs;

  llvm::BasicBlock *bblock;
  llvm::BasicBlock *endblock;
  llvm::AllocaInst *returnAlloca;

  CodegenBlockContext(llvm::BasicBlock *bb):bblock(bb){}
  CodegenBlockContext(){}
};

class CodegenContext {
  CodegenBlockContext *currentBlock(){
    if(block.size()!=0)
      return block.back();
    return &global;
  }
 public:

  bool import_compiler = false;
  llvm::LLVMContext context;
  //std::unique_ptr<llvm::Module> module;
  llvm::Module *module;
  llvm::IRBuilder<> *builder;
  CodegenBlockContext global;
  std::vector<CodegenBlockContext *> block;
  llvm::Function *mainF;
  std::vector<std::vector<Statement *> *> defered;

  std::tuple<llvm::AllocaInst *, VariableDecl *> *getVariable(std::string *name);
  llvm::AllocaInst *getVariableAlloca(std::string *name);
  VariableDecl *getVariableDecl(std::string *name);
  void setVariable(std::string *name, llvm::AllocaInst *var, VariableDecl *vd);

  std::tuple<llvm::Type *, StructStatement *> *getStruct(std::string *name);
  llvm::Type* getStructType(std::string *name);
  StructStatement *getStructStruct(std::string *name);
  void setStruct(std::string *name, llvm::Type *t, StructStatement *st);

  llvm::BasicBlock *getEndBlock();
  llvm::AllocaInst *getReturnAlloca();

  CodegenContext() {}
};

CodegenContext *codegen(std::vector<Statement *> *statements,
                        std::string outfile, bool print, bool ofile=true);
