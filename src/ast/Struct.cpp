#include "Struct.h"
#include "../base.h"
#include "../LLContext.h"
#include "Named.h"
#include "Type.h"
#include "Expression.h"
#include "Method.h"
#include "Interface.h"

StructType::StructType(std::string id)
  : id(id)
{}

// TODO methods prepare and compile?

void StructType::prepare(CC *cc){
  // TODO
  // TODO check for name clashes
  Named *named = new Named();
  named->id = id;
  named->type = this;
  named->alloca = 0;
  named->isType = true;
  cc->registerNamed(id, named);
}

void StructType::compile(CC *cc){
  // TODO do I want anything here?
  // TODO make sure types are valid
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
      mem_t.push_back((llvm::Type *)it->generatedType); // TODO is this ok?
    } else {
      mem_t.push_back((llvm::Type *)m->type->typegen(cc));
    }
  }

  ((llvm::StructType *)generatedType)->setBody(mem_t);
}

bool StructType::hasOp(CC *cc, std::string op, Expression *rhs){
  // TODO
  return false;
}
void* StructType::opgen(CC *cc, Expression *lhs, std::string op, Expression *rhs){
  // TODO
  return 0;
}
Type* StructType::optyperesolve(CC *cc, std::string op, Expression *rhs){
  // TODO
  return 0;
}
Compatibility StructType::compatible(CC *cc, Type *t){
  // TODO
  return INCOMPATIBLE;
}
void* StructType::castgen(CC *cc, Expression *e){
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
