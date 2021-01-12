#include "Struct.h"
#include "../base.h"
#include "../LLContext.h"
#include "Named.h"
#include "Type.h"
#include "Expression.h"

StructType::StructType(std::string id)
  : id(id)
{}


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
}

void *StructType::typegen(CC *cc){
  // TODO
  if(generatedType) return generatedType;

  std::vector<llvm::Type *> mem_t;
  auto s = llvm::StructType::create(cc->llc->context, mem_t, id);

  for(auto m: members){
    mem_t.push_back((llvm::Type *)m->type->typegen(cc));
  }

  s->setBody(mem_t);

  generatedType = s;

  cc->lookup(id)->alloca = s;

  return s;
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
Compatibility StructType::compatible(Type *t){
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
