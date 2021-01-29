#include "MemAccess.h"
#include "../base.h"
#include "../LLContext.h"
#include "Enum.h"
#include "Named.h"
#include "VariableValue.h"
#include "ast.h"
#include <ostream>

void MemAccess::compile(CC *cc){
  exp->compile(cc);

  // are we an enum?
  if(VariableValue *v = dynamic_cast<VariableValue*>(exp)){
    Named *n = cc->lookup(v->id);
    // TODO take similar approach for static members of structs
    if(n->isType)
      if (Enum *e = dynamic_cast<Enum*>(n->type->baseType(cc))) {
        this->e = e;

        // makesure the member exists
        if(e->members.find(id) == e->members.end()){
          graceFulExit(dbg, "Enum " + e->id + " doesn't have member " + id);
        }

        return;
      }
  }

  // ensure id belongs to exp
  if(!exp->type(cc)->hasMem(cc, exp, id)){
    graceFulExit(dbg, "Expression of type " + exp->type(cc)->toString() +  " doesn't have member " + id);
  }
}

void MemAccess::prepare(CC *cc){
  exp->prepare(cc);
}

void *MemAccess::exprgen(CC *cc){
  if(e){
    return e->enumMemGen(cc, id);
  }
  return exp->type(cc)->memgen(cc, exp, id);
}

MemAccess::MemAccess(Expression *exp, std::string id)
  : exp(exp)
  , id(id)
{}

Type *MemAccess::type(CC *cc){
  if(e) return e->innerType;
  return exp->type(cc)->memtyperesolve(cc, exp, id);
}

void *MemAccess::allocagen(CC *cc){
  if(e) return 0;
  return exp->type(cc)->memalloca(cc, exp, id);
}
