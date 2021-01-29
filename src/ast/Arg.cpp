#include "Arg.h"
#include "Named.h"
#include "../base.h"
#include "../LLContext.h"

void Arg::compile(CC *cc){
  Named *named = new Named();
  named->id = id;
  named->type = type;
  named->alloca = 0;
  cc->registerNamed(id, named);

  type->compile(cc);
}

void Arg::prepare(CC *cc){
  type->prepare(cc);
}

void Arg::codegen(CC *cc){
  Named *named = cc->lookup(id);
  auto alloca = cc->llc->builder->CreateAlloca((llvm::Type *)type->typegen(cc));
  named -> alloca = alloca;
}
