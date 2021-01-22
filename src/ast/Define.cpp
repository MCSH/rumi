#include "Define.h"
#include "Named.h"
#include "../base.h"
#include "../LLContext.h"
#include "Expression.h"
#include "Type.h"

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

  // TODO casting
}

void Define::codegen(CC *cc){
  auto named = cc->lookup(id);
  named -> alloca = cc->llc->builder->CreateAlloca((llvm::Type*)type->typegen(cc), 0, id.c_str());

  if(expression){
    cc->llc->builder->CreateStore((llvm::Value*)expression->exprgen(cc), (llvm::Value*)named->alloca);
  }
}

Define::Define(std::string id, Expression *e, Type *t)
  : id(id)
  , expression(e)
  , type(t)
{}

void Define::set(std::string key, void *value){
  if(key == "id"){
    id = std::string((char *) value);
    return;
  }

  if(key == "expression"){
    expression = (Expression *) value;
    return;
  }

  if(key == "type"){
    type = (Type *) value;
    return;
  }

  // TODO error?
}

void Define::add(std::string key, void *value){
  // TODO error?
}
