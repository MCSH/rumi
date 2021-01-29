#include "MemAccess.h"
#include "../base.h"
#include "../LLContext.h"
#include "Named.h"
#include "ast.h"
#include <ostream>

void MemAccess::compile(CC *cc){
  exp->compile(cc);
  // ensure id belongs to exp
  if(!exp->type(cc)->hasMem(cc, exp, id)){
    graceFulExit(dbg, "Expression of type " + exp->type(cc)->toString() +  " doesn't have member " + id);
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
