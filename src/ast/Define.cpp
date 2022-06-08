#include "Define.h"
#include "Named.h"
#include "../base.h"
#include "../LLContext.h"
#include "Expression.h"
#include "Type.h"
#include "VariableValue.h"
#include "Cast.h"

void Define::prepare(CC *cc){
  if(expression) expression->prepare(cc);
  if(type) type -> prepare(cc);
}

void Define::compile(CC *cc){
  if(expression) expression->compile(cc);
  if(!type){
    type = expression->type(cc);
  }
  type -> compile(cc);
  Named *named = new Named();
  named -> id = id;
  named -> type = type;
  named -> alloca = 0;
  cc->registerNamed(id, named);

  if(!type) // In cases where the variable is defined without any type, e.x.: var := 1;
    return;

  if(!expression) // In cases where the variable is defined without any value, e.x.: var :int;
    return;

  // TODO casting handle this together wi assign 
  auto compatible = type->compatible(cc, expression->type(cc)->baseType(cc));

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
    expression = new Cast(expression, type);
    expression->prepare(cc);
    expression->compile(cc);
  }
}

void Define::codegen(CC *cc){
  auto named = cc->lookup(id);
  named -> alloca = cc->llc->builder->CreateAlloca((llvm::Type*)type->typegen(cc), 0, id.c_str());

  if(expression){
    cc->llc->builder->CreateStore((llvm::Value*)expression->exprgen(cc), (llvm::Value*)named->alloca);
  } else {
    type->initgen(cc, new VariableValue(id));
  }
}

Define::Define(std::string id, Expression *e, Type *t)
  : id(id)
  , expression(e)
  , type(t)
{}

