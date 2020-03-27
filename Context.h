#pragma once

#include "BlockContext.h"
#include "nodes/Type.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <fstream>
#include <llvm/IR/Value.h>
#include <tuple>

class GlobalContext: public BlockContext{};

class VariableDecl;
class CompileStatement;
class InterfaceStatement;
class FunctionSignature;

// TODO memory leak
class Context{
  BlockContext *currentBlock(){
    if(blocks.size()!=0)
      return blocks.back();
    return &global;
  }
 public:
  GlobalContext global;
  std::vector<BlockContext *> blocks;

  std::vector<CompileStatement *> compiles; // TODO redundent?
  std::vector<Statement *> *codes; // TODO redundent?

  int struct_type_counter = 1;
  bool import_compiler = false;

  llvm::LLVMContext context;
  llvm::Module *module;
  llvm::IRBuilder<> *builder;
  llvm::Function *mainF;
  std::vector<std::vector<Statement *> *> defered;

  std::tuple<llvm::AllocaInst *, VariableDecl *> *getVariable(std::string *name);
  llvm::AllocaInst *getVariableAlloca(std::string *name);
  VariableDecl *getVariableDecl(std::string *name);
  void setVariable(std::string *name, llvm::AllocaInst *var, VariableDecl *vd);

  llvm::Type* getStructType(std::string *name);
  void setStruct(std::string *name, llvm::Type *t, StructStatement *st);

  llvm::Type* getInterfaceType(std::string *name);
  llvm::Type* getInterfaceVtableType(std::string *name);
  void setInterface(std::string *name, llvm::Type *t, llvm::Type *t2, InterfaceStatement *it);

  llvm::BasicBlock *getEndBlock();
  llvm::AllocaInst *getReturnAlloca();

  BlockContext *getBlock() {
    if (blocks.size() != 0)
      return blocks.back();
    return &global;
  }

  BlockContext *getParentBlock() {
    if(blocks.size() < 2)
      return &global;
    return blocks[blocks.size()-2];
  }

  Type *getVariableType(std::string *name){
    for(auto i = blocks.rbegin(); i!=blocks.rend(); i++){
      auto vars = (*i)->_vars;
      auto p = vars.find(*name);
      if(p!=vars.end())
        return p->second;
    }
    return global._vars[*name];
  }

  StructStatement *getStruct(std::string *name){
    for(auto i = blocks.rbegin(); i!=blocks.rend(); i++){
      auto vars = (*i)->structs;
      auto p = vars.find(*name);
      if(p!=vars.end())
        return p->second;
    }
    return global.structs[*name];
  }

  InterfaceStatement *getInterface(std::string *name){
    for(auto i = blocks.rbegin(); i!=blocks.rend(); i++){
      auto vars = (*i)->interfaces;
      auto p = vars.find(*name);
      if(p!=vars.end())
        return p->second;
    }
    return global.interfaces[*name];
  }

  FunctionSignature *getFunction(std::string *name){
    for(auto i = blocks.rbegin(); i!=blocks.rend(); i++){
      auto vars = (*i)->functions;
      auto p = vars.find(*name);
      if(p!=vars.end())
        return p->second;
    }
    return global.functions[*name];
  }

  FunctionDefine *getCurrentFunction(){
    for(auto i = blocks.rbegin(); i!=blocks.rend(); i++){
      auto f = (*i)->currentFunction;
      if(f)
        return f;
    }
    return NULL;
  }
};
