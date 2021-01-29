#include "Interface.h"
#include "Expression.h"
#include "FunctionSig.h"
#include "Named.h"
#include "../base.h"
#include "../LLContext.h"
#include "Struct.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalValue.h>
#include "Method.h"
#include "PointerType.h"
#include "PtrValue.h"
#include "ast.h"

Interface::Interface(std::string id)
  : id(id)
{}

void Interface::addMethod(CC *cc, FunctionSig *m){
  // TODO name clash?
  methods[m->id] = m;
}

FunctionSig *Interface::resolveMethod(CC *cc, std::string methodId){
  // TODO
  auto ans = methods.find(methodId);
  if(ans == methods.end()){
    return 0;
  }

  return ans->second;
}

void *Interface::typegen(CC *cc){
  return generatedTypePtr;
}

void Interface::compile(CC *cc){
  // create type
  std::vector<llvm::Type *> vptr_t;
  for(auto m: methods){
    vptr_t.push_back(llvm::Type::getInt64PtrTy(cc->llc->context));
  }

  llvm::Type *generatedVptr = llvm::StructType::create(cc->llc->context, vptr_t, id + "$$_vptr");
  this->generatedVptr = generatedVptr;
  // create vptr
  std::vector<llvm::Type *> int_t;
  int_t.push_back(generatedVptr->getPointerTo()); // vptr ptr
  int_t.push_back(llvm::Type::getInt64Ty(cc->llc->context)); // offset to base obj

  auto generatedType = llvm::StructType::create(cc->llc->context, int_t, id);
  this->generatedType = generatedType;

  generatedTypePtr = generatedType -> getPointerTo();

  cc->lookup(id)->alloca = generatedTypePtr;
}

void Interface::prepare(CC *cc){
  Named *named = new Named();
  named->id = id;
  named->type = this;
  named->alloca = 0;
  named->isType = true;
  cc->registerNamed(id, named);
}

bool Interface::hasOp(CC *cc, std::string op, Expression *rhs){
  return false;
}

void *Interface::opgen(CC *cc, Expression *lhs, std::string op, Expression *rhs){
  return 0;
}

Type *Interface::optyperesolve(CC *cc, std::string op, Expression *rhs){
  return 0;
}

Compatibility Interface::compatible(CC *cc, Type *t){

  // handle pointer
  if(PointerType *pt = dynamic_cast<PointerType *>(t)){
    t = pt->innerType->baseType(cc);
  }

  // handle structs
  if(StructType *st = dynamic_cast<StructType *>(t)){
    if(hasImplemented(cc, st)){
      return ImpCast;
    }
  }

  if(t == this) return OK;
  
  return INCOMPATIBLE;
}

void *Interface::castgen(CC *cc, Expression *e){

  // TODO ensure e is a struct here onwards.
  if(dynamic_cast<Interface *>(e->type(cc)->baseType(cc))){
    return e->exprgen(cc);
  }

  // Dealing with a pointer
  if(PointerType *pt = dynamic_cast<PointerType *>(e->type(cc)->baseType(cc))){
    e = new PtrValue(e);
  }

  StructType *st = (StructType*) e->type(cc)->baseType(cc);
  auto salloca = (llvm::Value *) e->allocagen(cc);

  // call resolveInterfaceVptrInd
  int vptr_ind = st->resolveInterfaceVptrInd(cc, this);

  auto vptr_ref = cc->llc->builder->CreateStructGEP(
      (llvm::Type *)st->typegen(cc), salloca, vptr_ind,
      "vptr_ref");

  auto vtable = (llvm::Value *) resolveVptr(cc, st); // Global vptr

  // ensure vptr is initialized
  cc->llc->builder->CreateStore(vtable, cc->llc->builder->CreateStructGEP((llvm::Type *)generatedType, vptr_ref, 0));

  // ensure vptr offset is correctly set
  auto offset = cc->llc->builder->CreateSub(salloca, vptr_ref);
  cc->llc->builder->CreateStore(offset, cc->llc->builder->CreateStructGEP((llvm::Type *)generatedType, vptr_ref, 1));

  return vptr_ref; // TODO load?
}

bool Interface::hasMem(CC *cc, Expression *exp, std::string id){
  return false;
}

void *Interface::memgen(CC *cc, Expression *exp, std::string id){
  return 0;
}

Type *Interface::memtyperesolve(CC *cc, Expression *exp, std::string id){
  return 0;
}

void *Interface::memalloca(CC *cc, Expression *exp, std::string id){
  return 0;
}


bool Interface::hasImplemented(CC *cc, StructType *st){
  if(structs.find(st->id) != structs.end()){
    // We have previously made sure that this is implementing us
    return true;
  }

  for(auto m: methods){
    bool found = false;
    for(auto mm: st->methods){
      if(m.first == mm.first){
        // names matches
        // TODO ensure type matches as well
        found = true;
        break;
      }
    }

    if(!found){
      cc->debug(NONE) << "Struct " << st->id <<" is missing method " << m.first << " for interface " << this->id << std::endl;
      return false;
    }
  }

  structs.insert(st->id);
  return true;
}


void *Interface::resolveVptr(CC *cc, StructType *st){
  auto name = id + "$$" + st->id;
  if(auto gb = cc->llc->module->getNamedGlobal(name)){
    return gb;
  }

  auto vt = ((llvm::StructType *) generatedVptr)->create(cc->llc->context);
  cc->llc->module->getOrInsertGlobal(name, vt);
  auto vtable = cc->llc->module->getNamedGlobal(name);
  vtable->setLinkage(llvm::GlobalValue::ExternalLinkage);

  std::vector<llvm::Constant *> initVtable;

  for(auto m: methods){
    llvm::Function * mf = cc->llc->module->getFunction(st->resolveMethod(cc, m.second->id)->f->id);

    auto c = llvm::ConstantExpr::getBitCast(
            mf, llvm::Type::getInt64PtrTy(cc->llc->context));

    initVtable.push_back(c);
  }
  
  vtable->setInitializer(llvm::ConstantStruct::getAnon(initVtable, generatedVptr));

  return vtable;
}


int Interface::methodInd(CC *cc, std::string mid){
  int i = 0;
  for(auto m: methods){
    if(m.first == mid){
      return i;
    }
    i++;
  }

  graceFulExit(dbg, "couldn't find method " + mid + " on interface " + id);
  return 0;
}


bool Interface::hasPreOp(CC *cc, std::string op){
  return false;
}

Type *Interface::preoptyperesolve(CC *cc, std::string op){
  return 0;
}

void *Interface::preopgen(CC *cc, std::string op, Expression *value){
  return 0;
}

std::string Interface::toString(){
  return id;
}
