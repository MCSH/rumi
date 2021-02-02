#include "../base.h"
#include "../LLContext.h"
#include "Enum.h"
#include "Named.h"
#include "PrimitiveType.h"
#include "Type.h"
#include <llvm/IR/Constants.h>

void Enum::addMember(CC *cc, EnumMember *m){
  // name clash?

  if(members.find(m->key) != members.end()){
    graceFulExit(dbg, "Enum has repeated definition for member " + m->key);
  }
  
  if(!inited){
    if(m->hasNum) last = m->num -1;
    else last = 0;
    inited = true;
  }
  members[m->key] = m;
  if(m->hasNum){
    // Make sure it's bigger than the last one
    if(last >= m->num){
      graceFulExit(dbg, "Enum member " + m->key + " has a value less than the previous one.\nMaybe change their order?");
    }
    last = m->num;
  } else {
    last ++;
    m->num = last;
  }

  memberOrders.push_back(m->key);
}

Enum::Enum(){
  innerType = new PrimitiveType(t_s64);
}

std::string Enum::toString(){
  return "Enum " + id;
}

void *Enum::typegen(CC *cc){
  return innerType->typegen(cc);
}

void Enum::compile(CC *cc){
  // register
  Named *named = new Named();
  named->id = id;
  named->type = this;
  named->alloca = 0;
  named->isType = true;
  cc->registerNamed(id, named);
  innerType->compile(cc);
}

void Enum::prepare(CC *cc){
  innerType->prepare(cc);
}

bool Enum::hasOp(CC *cc, std::string op, Expression *rhs){
  return innerType->hasOp(cc, op, rhs);
}

void *Enum::opgen(CC *cc, Expression *lhs, std::string op, Expression *rhs){
  return innerType->opgen(cc, lhs, op, rhs);
}

Type *Enum::optyperesolve(CC *cc, std::string op, Expression *rhs){
  return innerType->optyperesolve(cc, op, rhs);
}

Compatibility Enum::compatible(CC *cc, Type *t){
  return innerType->compatible(cc, t);
}

void *Enum::castgen(CC *cc, Expression *e){
  return innerType->castgen(cc, e);
}

bool Enum::hasMem(CC *cc, Expression *exp, std::string id){
  return innerType->hasMem(cc, exp, id);
}

void *Enum::memgen(CC *cc, Expression *exp, std::string id){
  return innerType->memgen(cc, exp, id);
}

Type *Enum::memtyperesolve(CC *cc, Expression *exp, std::string id){
  return innerType->memtyperesolve(cc, exp, id);
}

void *Enum::memalloca(CC *cc, Expression *exp, std::string id){
  return innerType->memalloca(cc, exp, id);
}

bool Enum::hasPreOp(CC *cc, std::string op){
  return innerType->hasPreOp(cc, op);
}

Type *Enum::preoptyperesolve(CC *cc, std::string op){
  return innerType->preoptyperesolve(cc, op);
}

void *Enum::preopgen(CC *cc, std::string op, Expression *value){
  return innerType->preopgen(cc, op, value);
}

void *Enum::enumMemGen(CC *cc, std::string id){
  int value = members[id]->num;
  return llvm::ConstantInt::get((llvm::Type *)innerType->typegen(cc), value, true);
}

void Enum::initgen(CC *cc, Expression *alloca){
  // Do nothing
}

bool Enum::hasIndex(CC *cc, Expression *index){
  return 0;
}

void *Enum::indexgen(CC *cc, Expression *expr, Expression *index){
  return 0;
}

Type *Enum::indextyperesolve(CC *cc, Expression *index){
  return 0;
}

void *Enum::indexallocagen(CC *cc, Expression *expr, Expression *index){
  return 0;
}
