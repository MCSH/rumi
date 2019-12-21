#pragma once
#include <fstream>
#include "node.h"
#include <map>

std::vector<Statement *>* compile(char *fileName);

    // TODO memory leak
class BlockContext { // Block Context
public:
  // Type *returnType; // TODO move this to function define?
  std::map<std::string, Type *> vars;
  std::map<std::string, StructStatement *> structs;
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

  void newFunction(std::string *name, FunctionSignature *fs){
    // TODO check for name collision
    // TODO maybe register as var?
    functions[*name] = fs;
  }
};

    // TODO memory leak
class CompileContext{
public:
  BlockContext global;
  std::vector<BlockContext *> blocks;

  BlockContext *getBlock() {
    if(blocks.size()!=0)
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
