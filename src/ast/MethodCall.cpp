#include "MethodCall.h"
#include "Arg.h"
#include "FunctionSig.h"
#include "PointerType.h"
#include "PtrValue.h"
#include "Struct.h"
#include "../base.h"
#include "../LLContext.h"
#include "Address.h"

void *MethodCall::exprgenInterface(CC *cc){
  auto e = exp->exp;
  // TODO make sure we are not pointers

  if(dynamic_cast<PointerType *>(e->type(cc)->baseType(cc))){
    // TODO make sure the level is only 1
    e = new PtrValue(e);
    e->prepare(cc);
    e->compile(cc);
  }
  
  auto ptr = (llvm::Value *)e->exprgen(cc);
  cc->debug(NONE) << "ptr is " << ptr << std::endl;
  auto vtable = cc->llc->builder->CreateStructGEP((llvm::Type *)i->generatedType, ptr, 0);

  // TODO resolve fptr
  auto mi = i->methodInd(cc, fs->id);
  auto fptr = cc->llc->builder->CreateStructGEP((llvm::Type *) i->generatedVptr, vtable, mi);

  // TODO resolve and add self to arguments
  auto offset = cc->llc->builder->CreateStructGEP((llvm::Type *) i->generatedType, ptr, 1);
  offset = cc->llc->builder->CreateLoad(offset);
  auto self = cc->llc->builder->CreateAdd(ptr, offset);

  // TODO setup fcall
  std::vector<llvm::Value *> params;
  params.push_back(self);
  for(auto e: args){
    params.push_back((llvm::Value *)e->exprgen(cc));
  }

  llvm::FunctionType * ftype;
  {
    auto rt = (llvm::Type *) fs->returnType->typegen(cc);
    std::vector<llvm::Type *> types;
    for(Arg *a: fs->args) types.push_back((llvm::Type *) a->type->typegen(cc));
    ftype = llvm::FunctionType::get(rt, types, false); // vararg?
  }

  fptr = cc->llc->builder->CreateLoad(fptr);
  fptr = cc->llc->builder->CreateLoad(fptr);

  return cc->llc->builder->CreateCall(ftype, fptr, params);
}

void MethodCall::compileInterface(CC *cc){
  
  auto bt = exp->exp->type(cc)->baseType(cc);
  i = dynamic_cast<Interface *>(bt);
  if(!i){
    if(PointerType *pt = dynamic_cast<PointerType *>(bt)){
      i = dynamic_cast<Interface *>(pt->innerType->baseType(cc));
    }

    if(!i){
      cc->debug(NONE) << "Can't call method on non-structs" << std::endl;
      exit(1);
    }
  }

  fs = i->resolveMethod(cc, exp->id);
  if(!fs){
    cc->debug(NONE) << "Can't find method " << exp->id << " in interface " << i->id << std::endl;
    exit(1);
  }

  for(auto e: args) e->compile(cc);

  // TODO casting
}

void MethodCall::compile(CC *cc){
  // TODO
  exp->exp->compile(cc);

  auto bt = exp->exp->type(cc)->baseType(cc);
  s = dynamic_cast<StructType *>(bt);
  if(!s){
    if(PointerType *pt = dynamic_cast<PointerType *>(bt)){
      s = dynamic_cast<StructType *>(pt->innerType->baseType(cc));
    }

    if(!s){
      return compileInterface(cc);
    }
  }

  method = s->resolveMethod(cc, exp->id);
  if(!method){
    cc->debug(NONE) << "Can't find method " << exp->id << " in struct " << s->id << std::endl;
    exit(1);
  }

  // add self to arguments
  auto base = exp->exp;
  if(PointerType *pt = dynamic_cast<PointerType *>(base->type(cc))){
    // TODO make sure its only one level deep 
  } else {
    base = new Address(base);
    base->prepare(cc);
    base->compile(cc);
  }

  args.insert(args.begin(), base);

  auto fname = s->id + "$" + exp->id;
  // setup fcall
  fcall = new FCall();
  fcall->id = fname;
  fcall->args = args;

  fcall->prepare(cc);
  fcall->compile(cc);
}
void MethodCall::prepare(CC *cc){
  exp->exp->prepare(cc);
  for(auto e: args) e->prepare(cc);
}
void* MethodCall::exprgen(CC *cc){
  if(i) return exprgenInterface(cc);
  return fcall->exprgen(cc);
}
Type* MethodCall::type(CC *cc){
  if(i) return fs->returnType;
  return method->f->returnType;
}
void* MethodCall::allocagen(CC *cc){
  return 0;
}

MethodCallStmt::MethodCallStmt(MethodCall *mc)
  : mc(mc)
{}

void MethodCallStmt::compile(CC *cc){
  mc->compile(cc);
}
void MethodCallStmt::prepare(CC *cc){
  mc->prepare(cc);
}
void MethodCallStmt::codegen(CC *cc){
  mc->exprgen(cc);
}
