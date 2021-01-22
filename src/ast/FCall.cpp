#include "FCall.h"
#include "../base.h"
#include "../LLContext.h"
#include "FunctionType.h"
#include "Named.h"

void FCall::compile(CC *cc){
  // ensure function exists
  if(!cc->lookup(id)){
    cc->debug(NONE) << "Couldn't find function " << id << std::endl;
    exit(1);
  }
  // TODO ensure the arguments match the type
  for(auto e: args) e->compile(cc);
}

void FCall::prepare(CC *cc){
  for(auto e: args) e->prepare(cc);
}

Type *FCall::type(CC *cc){
  Named *n = cc->lookup(id);
  FunctionType *ft = dynamic_cast<FunctionType *>(n->type);
  // TODO this is an error
  if(!ft) return 0;
  return ft->returnType;
}

void *FCall::exprgen(CC *cc){
  llvm::Function *f = (llvm::Function*)cc->lookup(id)->alloca;
  std::vector<llvm::Value *> params;
  for(auto e: args){
    params.push_back((llvm::Value *)e->exprgen(cc));
  }
  return cc->llc->builder->CreateCall(f->getFunctionType(), f, params);
}

void *FCall::allocagen(CC *cc){
  cc->debug(NONE) << "Calling alloca on unsupported type" << std::endl;
  exit(1);
}

void FCall::set(std::string key, void *value){
  if(key == "id"){
    id = std::string((char *) value);
    return;
  }

  // TODO error?
}

void FCall::add(std::string key, void *value){
  if(key == "arg"){
    args.push_back((Expression *)value);
    return;
  }
  // TODO error?
}

void FCallStmt::compile(CC *cc){
  fc.compile(cc);
  // TODO casting
}
void FCallStmt::prepare(CC *cc){
  fc.prepare(cc);
}
void FCallStmt::codegen(CC *cc){
  fc.exprgen(cc);
}

void FCallStmt::set(std::string key, void *value){
  fc.set(key, value);
}

void FCallStmt::add(std::string key, void *value){
  fc.add(key, value);
}
