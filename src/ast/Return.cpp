#include "Return.h"
#include "../base.h"
#include "../LLContext.h"

// TODO computational graph

Return::Return(Expression *value)
  : value(value)
{
}

void Return::compile(CC *cc){
  if(value) value->compile(cc);
}

void Return::prepeare(CC *cc){
  if(value) value->prepeare(cc);
}

void Return::codegen(CC *cc){
  // TODO
  if(!value){
    cc->llc->builder->CreateRetVoid();
  } else {
    // TODO
    cc->llc->builder->CreateRet((llvm::Value *)value->exprgen(cc));
  }
}
