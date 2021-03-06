#include "NamedType.h"
#include "Named.h"
#include "../base.h"
#include "ast.h"

NamedType::NamedType(std::string id)
  : id(id)
{}

void NamedType::prepare(CC *cc){
}

void NamedType::compile(CC *cc){
  Named *named = cc->lookup(id);
  if(!named || !named->isType){
    graceFulExit(dbg, id + " is not a type!");
  }

  resolvedType = named->type;
}

void* NamedType::typegen(CC *cc){
  return resolvedType->typegen(cc);
}

bool NamedType::hasOp(CC *cc, std::string op, Expression *rhs){
  return resolvedType->hasOp(cc, op, rhs);
}

void *NamedType::opgen(CC *cc, Expression *lhs, std::string op, Expression *rhs){
  return resolvedType->opgen(cc, lhs, op, rhs);
}

Type* NamedType::optyperesolve(CC *cc, std::string op, Expression *rhs){
  return resolvedType->optyperesolve(cc, op, rhs);
}

Compatibility NamedType::compatible(CC *cc, Type *t){
  return resolvedType->compatible(cc, t);
}

void* NamedType::castgen(CC *cc, Expression *e){
  return resolvedType->castgen(cc, e);
}

bool NamedType::hasMem(CC *cc, Expression *exp, std::string id){
  return resolvedType->hasMem(cc, exp, id);
}

void *NamedType::memgen(CC *cc, Expression *exp, std::string id){
  return resolvedType->memgen(cc, exp, id);
}

void *NamedType::memalloca(CC *cc, Expression *exp, std::string id){
  return resolvedType->memalloca(cc, exp, id);
}

Type *NamedType::memtyperesolve(CC *cc, Expression *exp, std::string id){
  return resolvedType->memtyperesolve(cc, exp, id);
}

Type *NamedType::baseType(CC *cc){
  return resolvedType->baseType(cc);
}

bool NamedType::hasPreOp(CC *cc, std::string op){
  return resolvedType->hasPreOp(cc, op);
}

Type *NamedType::preoptyperesolve(CC *cc, std::string op){
  return resolvedType->preoptyperesolve(cc, op);
}

void *NamedType::preopgen(CC *cc, std::string op, Expression *value){
  return resolvedType->preopgen(cc, op, value);
}

std::string NamedType::toString(){
  return id;
}

void NamedType::initgen(CC *cc, Expression *alloca){
  resolvedType -> initgen(cc, alloca);
}

bool NamedType::hasIndex(CC *cc, Expression *index){
  return resolvedType->hasIndex(cc, index);
}

void *NamedType::indexgen(CC *cc, Expression *expr, Expression *index){
  return resolvedType->indexgen(cc, expr, index);
}

void *NamedType::indexallocagen(CC *cc, Expression *expr, Expression *index){
  return resolvedType->indexallocagen(cc, expr, index);
}

Type *NamedType::indextyperesolve(CC *cc, Expression *index){
  return resolvedType->indextyperesolve(cc, index);
}
