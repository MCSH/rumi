#include "Assign.h"
#include "Expression.h"
#include "../base.h"
#include "../LLContext.h"
#include "Named.h"
#include "Type.h"

Assign::Assign(Expression *baseExpr, Expression *e)
  : baseExpr(baseExpr)
  , expression(e)
{
}

void Assign::prepare(CC *cc){
  expression->prepare(cc);
  baseExpr->prepare(cc);
}

void Assign::compile(CC *cc){
  expression->compile(cc);
  baseExpr->compile(cc);

  // TODO take care of casting
  auto compatible = baseExpr->type(cc)->compatible(cc, expression->type(cc)->baseType(cc));

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
    casting = true;
  } else {
    casting = false;
  }
}

void Assign::codegen(CC *cc){
  // get alloca
  // genereate value
  llvm::Value *v;
  if(casting){
    v = (llvm::Value *)baseExpr->type(cc)->castgen(cc, expression);
  } else {
    v = (llvm::Value *)expression->exprgen(cc);
  }
  // create store
  cc->llc->builder->CreateStore(v, (llvm::Value *)baseExpr->allocagen(cc));
}
