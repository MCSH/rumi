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
