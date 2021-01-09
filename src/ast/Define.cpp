#include "Define.h"
#include "Named.h"
#include "../base.h"
#include "../LLContext.h"
#include "Expression.h"
#include "Type.h"

// TODO prepreare and compile

void Define::prepeare(CC *cc){
}

void Define::compile(CC *cc){
  Named *named = new Named();
  named -> id = id;
  named -> type = type;
  named -> alloca = 0;
  cc->registerNamed(id, named);
}

void Define::codegen(CC *cc){
  // TODO
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
