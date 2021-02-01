#include "Struct.h"
#include "../base.h"
#include "../LLContext.h"
#include "BinOpDef.h"
#include "Named.h"
#include "PointerType.h"
#include "PtrValue.h"
#include "Type.h"
#include "Expression.h"
#include "Method.h"
#include "Interface.h"
#include "Address.h"
#include "FCall.h"

StructType::StructType(std::string id)
  : id(id)
{}

void StructType::prepare(CC *cc){
  // TODO check for name clashes
  Named *named = new Named();
  named->id = id;
  named->type = this;
  named->alloca = 0;
  named->isType = true;
  cc->registerNamed(id, named);
}

void StructType::compile(CC *cc){
  for(auto m: members){
    m->type->compile(cc);
  }
}

void *StructType::typegen(CC *cc){
  if(generatedType) return generatedType;

  std::vector<llvm::Type *> mem_t;
  auto s = llvm::StructType::create(cc->llc->context, mem_t, id);
  generatedType = s;

  resetBody(cc);

  cc->lookup(id)->alloca = s;

  return s;
}

void StructType::resetBody(CC *cc){
  
  std::vector<llvm::Type *> mem_t;
  for(auto m: members){
    auto id = &m->id;
    if(id->size() > 5 && id->substr(0, 5) == "vptr$"){
      auto it = (Interface *) m->type;
      mem_t.push_back((llvm::Type *)it->generatedType);
    } else {
      mem_t.push_back((llvm::Type *)m->type->typegen(cc));
    }
  }

  ((llvm::StructType *)generatedType)->setBody(mem_t);
}

bool StructType::hasOp(CC *cc, std::string op, Expression *rhs){
  // check for rhs type
  auto name = id + "." + op + "." + rhs->type(cc)->toString();
  BinOpDef *bod = 0;
  if(binops.find(name) != binops.end())
    bod = binops[name];
  // check for pointer to rhs type
  else if(PointerType *pt = dynamic_cast<PointerType *>(rhs->type(cc))){
    auto altname = id + "." + op + "." + pt->innerType->toString();
    if (binops.find(altname) != binops.end())
      bod = binops[altname];
  }
  if(bod) return true;
  return false;
}
// NOTE: This function doesn't respect prepare/compile for fcall and address
void* StructType::opgen(CC *cc, Expression *lhs, std::string op, Expression *rhs){
  bool rhsIsPointer = false;
  auto name = id + "." + op + "." + rhs->type(cc)->toString();
  BinOpDef *bod = 0;
  if(binops.find(name) != binops.end())
    bod = binops[name];
  // check for pointer to rhs type
  else if(PointerType *pt = dynamic_cast<PointerType *>(rhs->type(cc))){
    rhsIsPointer = true;
    auto altname = id + "." + op + "." + pt->innerType->toString();
    if (binops.find(altname) != binops.end())
      bod = binops[altname];
  }
  if(!bod) return 0;
  if(rhsIsPointer) rhs = new PtrValue(rhs);
  if(bod->firstPtr){
    lhs = new Address(lhs);
  }
  if(bod->secondPtr){
    rhs = new Address(rhs);
  }

  FCall fc;
  fc.id = bod->f->id;
  fc.args.push_back(lhs);
  fc.args.push_back(rhs);
  return fc.exprgen(cc);
}
Type* StructType::optyperesolve(CC *cc, std::string op, Expression *rhs){
  // check for rhs type
  auto name = id + "." + op + "." + rhs->type(cc)->toString();
  BinOpDef *bod = 0;
  if(binops.find(name) != binops.end())
    bod = binops[name];
  // check for pointer to rhs type
  else if(PointerType *pt = dynamic_cast<PointerType *>(rhs->type(cc))){
    auto altname = id + "." + op + "." + pt->innerType->toString();
    if (binops.find(altname) != binops.end())
      bod = binops[altname];
  }
  if(bod) return bod->f->returnType;
  return 0;
}
Compatibility StructType::compatible(CC *cc, Type *t){
  // TODO
  if(StructType *st = dynamic_cast<StructType*>(t->baseType(cc))){
    if(st == this) return OK;
  }
  return INCOMPATIBLE;
}
void* StructType::castgen(CC *cc, Expression *e){
  if(StructType *st = dynamic_cast<StructType*>(e->type(cc)->baseType(cc))){
    if(st == this) return e->exprgen(cc);
  }
  // TODO
  return 0;
}

int StructType::findmemindex(CC *cc, Expression *exp, std::string id){
  for(int i = 0 ; i < members.size(); i++){
    if(members[i]->id == id){
      return i;
    }
  }

  return -1;
}

bool StructType::hasMem(CC *cc, Expression *exp, std::string id){
  return findmemindex(cc, exp, id) != -1;
}

void *StructType::memgen(CC *cc, Expression *exp, std::string id){
  auto alloc = (llvm::Value *) memalloca(cc, exp, id);
  if(!alloc) return 0;
  return cc->llc->builder->CreateLoad(alloc);
}

Type *StructType::memtyperesolve(CC *cc, Expression *exp, std::string id){
  int ind = findmemindex(cc, exp, id);
  if(ind == -1) return 0;
  return members[ind]->type;
}

void *StructType::memalloca(CC *cc, Expression *exp, std::string id){
  int ind = findmemindex(cc, exp, id);
  if(ind == -1) return 0;

  std::vector<llvm::Value *> indices(2);
  indices[0] = llvm::ConstantInt::get(cc->llc->context, llvm::APInt(32, 0, true));
  indices[1] = llvm::ConstantInt::get(cc->llc->context, llvm::APInt(32, ind, true));

  auto alloc = (llvm::Value *)exp->allocagen(cc);

  // TODO check sanity
  return cc->llc->builder->CreateInBoundsGEP(alloc, indices);
}

void StructType::addMethod(CC *cc, Method *m){
  methods[m->methodName] = m;

  if(m->methodName == "!new"){
    initializer = m->f;
  }
}

void StructType::addBinOp(CC *cc, BinOpDef *bod){
  binops[bod->methodName] = bod;
}

Method *StructType::resolveMethod(CC *cc, std::string id){
  auto ans = methods.find(id);
  if(ans == methods.end()) return 0;
  return ans->second;
}

int StructType::resolveInterfaceVptrInd(CC *cc, Interface *it){
  auto name = "vptr$" + it->id;
  int i = 0;
  for(auto mem: members){
    if(mem->id == name){
      return i;
    }
    i++;
  }

  // Add vptr and regen body
  Define *d = new Define(name, 0, it); // type is handled in resetbody via substr match
  members.push_back(d);

  resetBody(cc);

  return i;
}

bool StructType::hasPreOp(CC *cc, std::string op){
  return false;
}

Type *StructType::preoptyperesolve(CC *cc, std::string op){
  return 0;
}

void *StructType::preopgen(CC *cc, std::string op, Expression *value){
  return 0;
}

std::string StructType::toString(){
  return id;
}

void StructType::initgen(CC *cc, Expression *alloca){
  // TODO
  if(!initializer) return;

  FCallStmt *fcs = new FCallStmt();

  // TODO check this
  alloca = new Address(alloca);

  // Create the function call
  fcs->fc.id = initializer->id;
  fcs->fc.args.push_back(alloca);

  fcs->prepare(cc);
  fcs->compile(cc);
  fcs->codegen(cc);
}
