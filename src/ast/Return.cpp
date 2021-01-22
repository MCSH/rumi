#include "Return.h"
#include "../base.h"
#include "../LLContext.h"

Return::Return(Expression *value)
  : value(value)
{
}

void Return::compile(CC *cc){
  if(value) value->compile(cc);
}

void Return::prepare(CC *cc){
  if(value) value->prepare(cc);
}

void Return::codegen(CC *cc){
  if(!value){
    cc->llc->builder->CreateRetVoid();
  } else {
    cc->llc->builder->CreateRet((llvm::Value *)value->exprgen(cc));
  }
}

void Return::set(std::string key, void *value){
  if(key == "value"){
    if(!value) value = 0;
    else value = (Expression *) value;
    return;
  }

  // TODO error?
}

void Return::add(std::string key, void *value){
  // TODO error?
}
