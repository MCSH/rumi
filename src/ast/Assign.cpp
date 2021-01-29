#include "Assign.h"
#include "Cast.h"
#include "Expression.h"
#include "../base.h"
#include "../LLContext.h"
#include "Named.h"
#include "Type.h"
#include "ast.h"

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
    graceFulExit(dbg, "Cannot explicitly cast types");
  }
  if(compatible == INCOMPATIBLE){
    // TODO add name of types
    graceFulExit(dbg, "Cannot cast types");
  }
  if(compatible == ImpCast){
    cc->debug(LOW) << "Casting implicitly" << std::endl;
    expression = new Cast(expression, baseExpr->type(cc));
    expression->prepare(cc);
    expression->compile(cc);
  }
}

void Assign::codegen(CC *cc){
  // get alloca
  // genereate value
  llvm::Value *v = (llvm::Value *)expression->exprgen(cc);
  // create store
  cc->llc->builder->CreateStore(v, (llvm::Value *)baseExpr->allocagen(cc));
}
