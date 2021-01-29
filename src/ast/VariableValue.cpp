#include "VariableValue.h"
#include "../base.h"
#include "../LLContext.h"
#include "Named.h"
#include <ostream>

void VariableValue::compile(CC *cc){
  // ensure variable exists
  if(!cc->lookup(id)){
    graceFulExit(dbg, "Couldn't find variable " + id);
  }
}

void VariableValue::prepare(CC *cc) {}

void *VariableValue::exprgen(CC *cc){
  return cc->llc->builder->CreateLoad((llvm::Value*)cc->lookup(id)->alloca);
}

VariableValue::VariableValue(std::string id)
  : id(id)
{}

Type *VariableValue::type(CC *cc){
  return cc->lookup(id)->type;
}

void *VariableValue::allocagen(CC *cc){
  return cc->lookup(id)->alloca;
}
