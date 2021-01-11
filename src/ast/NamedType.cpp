#include "NamedType.h"
#include "Named.h"
#include "../base.h"

NamedType::NamedType(std::string id)
  : id(id)
{}

void NamedType::prepare(CC *cc){
  // TODO
}

void NamedType::compile(CC *cc){
  Named *named = cc->lookup(id);
  if(!named || !named->isType){
    cc->debug(NONE) << id << " is not a type!" << std::endl;
    exit(1);
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

Compatibility NamedType::compatible(Type *t){
  return resolvedType->compatible(t);
}

void* NamedType::castgen(CC *cc, Expression *e){
  return resolvedType->castgen(cc, e);
}
