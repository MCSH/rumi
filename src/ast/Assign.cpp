#include "Assign.h"
#include "Expression.h"
#include "../base.h"
#include "../LLContext.h"
#include "Named.h"

Assign::Assign(std::string id, Expression *e)
  : id(id)
  , expression(e)
{
}

void Assign::prepeare(CC *cc){
  expression->prepeare(cc);
}

void Assign::compile(CC *cc){
  expression->compile(cc);
  auto v = cc->lookup(id);
  if(!v){
    cc->debug(NONE) << "Couldn't find variable " << id << std::endl;
    exit(1);
  }

  // TODO take care of casting
}

void Assign::codegen(CC *cc){
  // get alloca
  Named *name = cc->lookup(id);
  // genereate value
  llvm::Value *v = (llvm::Value *)expression->exprgen(cc);
  // create store
  cc->llc->builder->CreateStore(v, (llvm::Value *)name->alloca);
}
