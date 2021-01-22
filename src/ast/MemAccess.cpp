#include "MemAccess.h"
#include "../base.h"
#include "../LLContext.h"
#include "Named.h"
#include <ostream>

void MemAccess::compile(CC *cc){
  exp->compile(cc);
  // ensure id belongs to exp
  if(!exp->type(cc)->hasMem(cc, exp, id)){
    cc->debug(NONE) << "Expression doesn't have member " << id << std::endl;
    exit(1);
  }
}

void MemAccess::prepare(CC *cc){
  exp->prepare(cc);
}

void *MemAccess::exprgen(CC *cc){
  return exp->type(cc)->memgen(cc, exp, id);
}

MemAccess::MemAccess(Expression *exp, std::string id)
  : exp(exp)
  , id(id)
{}

Type *MemAccess::type(CC *cc){
  return exp->type(cc)->memtyperesolve(cc, exp, id);
}

void *MemAccess::allocagen(CC *cc){
  return exp->type(cc)->memalloca(cc, exp, id);
}

void MemAccess::set(std::string key, void *value){
  if(key == "exp"){
    exp = (Expression *) value;
    return;
  }

  if(key == "id"){
    id = std::string((char *) value);
    return;
  }

  // TODO error?
}

void MemAccess::add(std::string key, void *value){
  // TODO error?
}
