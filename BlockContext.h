#pragma once

#include "nodes/statements/InterfaceStatement.h"
#include "nodes/statements/StructStatement.h"
#include "nodes/statements/VariableDecl.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include <llvm/IR/Value.h>
#include <map>

class CompileDirective;

// TODO memory leak
class BlockContext { // Block Context
public:
  // Type *returnType; // TODO move this to function define?

  BlockContext *parent=0;

  // TODO merge
  std::map<std::string, Type *> _vars;
  std::map<std::string, std::tuple<llvm::AllocaInst *, VariableDecl *> *>
      variables;

  std::map<std::string, StructStatement *> structs;

  std::map<std::string, InterfaceStatement *> interfaces;

  std::map<std::string, FunctionSignature *> functions;

  std::map<std::string, CompileDirective *> directives;

  FunctionDefine *currentFunction;

  llvm::BasicBlock *bblock;
  llvm::BasicBlock *endblock = nullptr;
  llvm::AllocaInst *returnAlloca = nullptr;

  BlockContext() {}

  virtual ~BlockContext()=0;

  BlockContext(llvm::BasicBlock *bb) : bblock(bb) {}

  void newVar(std::string *name, Type *type) {
    // TODO check for name collision
    _vars[*name] = type;
  }

  void newDirective(std::string *name, CompileDirective *cd){
    directives[*name] = cd;
  }

  void newStruct(std::string *name, StructStatement *ss) {
    // TODO check for name collision
    // TODO maybe register as var?
    structs[*name] = ss;
  }

  void newInterface(std::string *name, InterfaceStatement *is) {
    // TODO check for name collision
    // TODO maybe register as var?
    interfaces[*name] = is;
  }

  void newFunction(std::string *name, FunctionSignature *fs);
};
