#pragma once

#include <fstream>
#include <map>
#include "nodes/Type.h"
#include "nodes/statements/StructStatement.h"
#include "nodes/statements/InterfaceStatement.h"
#include "nodes/statements/FunctionSignature.h"
#include "nodes/statements/CompileStatement.h"

#include "codegen.h"
#include "compiler.h"

    // TODO memory leak
class BlockContext { // Block Context
public:
  // Type *returnType; // TODO move this to function define?
  std::map<std::string, Type *> vars;
  std::map<std::string, StructStatement *> structs;
  std::map<std::string, InterfaceStatement *> interfaces;
  std::map<std::string, FunctionSignature *> functions;
  FunctionDefine *currentFunction;

  void newVar(std::string *name, Type *type){
    // TODO check for name collision
    vars[*name] = type;
  }

  void newStruct(std::string *name, StructStatement *ss){
    // TODO check for name collision
    // TODO maybe register as var?
    structs[*name] = ss;
  }

  void newInterface(std::string *name, InterfaceStatement *is){
    // TODO check for name collision
    // TODO maybe register as var?
    interfaces[*name] = is;
  }

  void newFunction(std::string *name, FunctionSignature *fs){
    // TODO check for name collision
    // TODO maybe register as var?
    functions[*name] = fs;
    newVar(name, fs->getType());
  }
};


    // TODO memory leak
class Context{
 public:
  BlockContext global;
  std::vector<BlockContext *> blocks;

  std::vector<CompileStatement *> compiles; // TODO redundent?
  std::vector<Statement *> *codes; // TODO redundent?

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
      auto vars = (*i)->vars;
      auto p = vars.find(*name);
      if(p!=vars.end())
        return p->second;
    }
    return global.vars[*name];
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
