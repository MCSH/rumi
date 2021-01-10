#include "Assign.h"
#include "Expression.h"
#include "../base.h"
#include "../LLContext.h"
#include "Named.h"
#include "Type.h"

Assign::Assign(std::string id, Expression *e)
  : id(id)
  , expression(e)
{
}

void Assign::prepare(CC *cc){
  expression->prepare(cc);
}

void Assign::compile(CC *cc){
  expression->compile(cc);
  auto v = cc->lookup(id);
  if(!v){
    cc->debug(NONE) << "Couldn't find variable " << id << std::endl;
    exit(1);
  }

  // TODO take care of casting
  auto compatible = v->type->compatible(expression->type(cc));

  if(compatible == ExpCast){
    // TODO add name of types
    cc->debug(NONE) << "Cannot explictly cast types" << std::endl;
    exit(1);
  }
  if(compatible == INCOMPATIBLE){
    // TODO add name of types
    cc->debug(NONE) << "Cannot cast types" << std::endl;
    exit(1);
  }
  if(compatible == ImpCast){
    cc->debug(LOW) << "Casting implicitly" << std::endl;
  }
}

void Assign::codegen(CC *cc){
  // get alloca
  Named *name = cc->lookup(id);
  // genereate value
  llvm::Value *v = (llvm::Value *)expression->exprgen(cc);
  // create store
  cc->llc->builder->CreateStore(v, (llvm::Value *)name->alloca);
}
