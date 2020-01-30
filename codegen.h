#pragma once
#include "nodes/statements/VariableDecl.h"
#include "nodes/statements/StructStatement.h"
#include "nodes/statements/InterfaceStatement.h"
#include "nodes/types/PointerType.h"
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
  std::map<std::string, std::tuple<std::tuple<llvm::Type *, llvm::Type*>*, InterfaceStatement *>*> interfaces;

  llvm::BasicBlock *bblock;
  llvm::BasicBlock *endblock=nullptr;
  llvm::AllocaInst *returnAlloca=nullptr;

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

  int struct_type_counter = 1;

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

  std::tuple<std::tuple<llvm::Type *, llvm::Type*>*, InterfaceStatement*> *getInterface(std::string *name);
  llvm::Type* getInterfaceType(std::string *name);
  llvm::Type* getInterfaceVtableType(std::string *name);
  InterfaceStatement *getInterfaceStatement(std::string *name);
  void setInterface(std::string *name, llvm::Type *t, llvm::Type *t2, InterfaceStatement *it);

  llvm::BasicBlock *getEndBlock();
  llvm::AllocaInst *getReturnAlloca();

  CodegenContext() {}
};

CodegenContext *codegen(std::vector<Statement *> *statements,
                        std::string outfile, bool print, bool ofile=true);
llvm::Value *castGen(Type *exprType, Type *baseType, llvm::Value *e, CodegenContext *cc, Node *n, bool expl, llvm::AllocaInst *alloc=nullptr);

void handleDefer(CodegenContext *cc);

llvm::Value* arrayToPointer(llvm::Type* t, llvm::Value * alloc, CodegenContext *cc);

