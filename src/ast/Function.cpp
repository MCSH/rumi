#include "Function.h"
#include "PrimitiveType.h"
#include "Statement.h"
#include <llvm/ADT/ArrayRef.h>
#include <llvm/IR/BasicBlock.h>
#include "../base.h"
#include "../LLContext.h"
#include "Type.h"
#include "FunctionType.h"
#include "Named.h"
#include "Arg.h"

bool isvoid(Type *rt){
  if(!rt) return true;
  if(PrimitiveType *pt = dynamic_cast<PrimitiveType *>(rt)){
    return pt->key == t_void || pt->key == t_unit;
  }
  return false;
}

void Function::prepare(CC *cc){
  cc->pushContext();
  for(Arg *a: args){
    a->prepare(cc);
  }

  returnType->prepare(cc);

  for(Statement *s: statements){
    s->prepare(cc);
  }

  b = cc->popContext();
}

void Function::compile(CC *cc){
  FunctionType *type = new FunctionType();
  type->returnType = returnType;

  Named *named = new Named();
  named -> id = id;
  named -> type = type;
  named -> alloca = 0;
  cc->registerNamed(id, named);

  cc->pushContext(b);
  for(Arg *a: args){
    a->compile(cc);
    type->args.push_back(a->type);
  }

  returnType->compile(cc);

  for(Statement *s: statements){
    s->compile(cc);
  }
  cc->popContext();
}

void Function::codegen(CC *cc){
  // TODO handle varargs
  cc->pushContext(b);
  llvm::Type* rt = returnType?(llvm::Type*)returnType->typegen(cc):
    llvm::Type::getVoidTy(cc->llc->context);

  std::vector<llvm::Type *> types;
  for(Arg *a: args) types.push_back((llvm::Type *)a->type->typegen(cc));

  auto ft = llvm::FunctionType::get(rt, types, false);

  auto f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, this->id, cc->llc->module);

  cc->llc->f = f;

  auto entry = llvm::BasicBlock::Create(cc->llc->context, "entry", f);
  cc->llc->builder->SetInsertPoint(entry);

  for(Arg *a: args){
    a->codegen(cc);
  }

  int i = 0;
  for(auto &a: f->args()){
    a.setName(args[i]->id);
    Named *arg_named = cc->lookup(args[i]->id);

    cc->llc->builder->CreateStore(&a, (llvm::Value *)arg_named->alloca);
    
    i++;
  }

  Named *named = cc->lookup(id);
  named->alloca = f;

  for(Statement *s: statements){
    s->codegen(cc);
  }
  cc->popContext();

  if(cc->llc->isBrOk()){
    // TODO check return type
    if(isvoid(returnType)){
      cc->llc->builder->CreateRetVoid();
    } else {
      graceFulExit(dbg, "Function " + id + " doesn't end with a return statement");
    }
  }
}
