#include "TypeDef.h"
#include "../base.h"
#include "Named.h"

TypeDef::TypeDef(std::string id, Type *type)
  : id(id)
  , type(type)
{}

std::string TypeDef::toString(){
  return id;
}

void* TypeDef::typegen(CC *cc){
  return type->typegen(cc);
}

void TypeDef::compile(CC *cc){
  type->compile(cc);
}

void TypeDef::prepare(CC *cc){
  type->prepare(cc);

  Named *named = new Named();
  named->id = id;
  named->type = type;
  named->alloca = 0;
  named->isType = true;
  cc->registerNamed(id, named);
}

void TypeDef::initgen(CC *cc, Expression *alloca){
  type->initgen(cc, alloca);
}

bool TypeDef::hasOp(CC *cc, std::string op, Expression *rhs){
  return type->hasOp(cc, op, rhs);
}

void* TypeDef::opgen(CC *cc, Expression *lhs, std::string op, Expression *rhs){
  return type->opgen(cc, lhs, op, rhs);
}
Type* TypeDef::optyperesolve(CC *cc, std::string op, Expression *rhs){
  return type->optyperesolve(cc, op, rhs);
}

Compatibility TypeDef::compatible(CC *cc, Type *t){
  // TODO: Might want to generate a warning over types of different kind
  return type->compatible(cc, t);
}

void* TypeDef::castgen(CC *cc, Expression *e){
  return type->castgen(cc, e);
}
  
bool TypeDef::hasMem(CC *cc, Expression *exp, std::string id){
  return type->hasMem(cc, exp, id);
}
void* TypeDef::memgen(CC *cc, Expression *exp, std::string id){
  return type->memgen(cc, exp, id);
}
Type* TypeDef::memtyperesolve(CC *cc, Expression *exp, std::string id){
  return type->memtyperesolve(cc, exp, id);
}
void* TypeDef::memalloca(CC *cc, Expression *exp, std::string id){
  return type->memalloca(cc, exp, id);
}

Type *TypeDef::baseType(CC *cc){
  return type->baseType(cc);
}

bool TypeDef::hasPreOp(CC *cc, std::string op){
  return type->hasPreOp(cc, op);
}

Type *TypeDef::preoptyperesolve(CC *cc, std::string op){
  return type->preoptyperesolve(cc, op);
}

void *TypeDef::preopgen(CC *cc, std::string op, Expression *value){
  return type->preopgen(cc, op, value);
}

bool TypeDef::hasIndex(CC *cc, Expression *index){
  return type->hasIndex(cc, index);
}
void *TypeDef::indexgen(CC *cc, Expression *expr, Expression *index){
  return type->indexgen(cc, expr, index);
}

Type *TypeDef::indextyperesolve(CC *cc, Expression *index){
  return type->indextyperesolve(cc, index);
}
void *TypeDef::indexallocagen(CC *cc, Expression *expr, Expression *index){
  return type->indexallocagen(cc, expr, index);
}
