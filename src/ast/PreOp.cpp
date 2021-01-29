#include "PreOp.h"
#include "../base.h"
#include "PointerType.h"
#include "ast.h"

PreOp::PreOp(std::string op, Expression *value)
  : op(op)
  , value(value)
{}

void PreOp::compile(CC *cc){
  value->compile(cc);

  if(!value->type(cc)->hasPreOp(cc, op)){
    graceFulExit(dbg, "Type doesn't support op " + op);
  }
}

void PreOp::prepare(CC *cc){
  value->prepare(cc);
}

Type *PreOp::type(CC *cc){
  return value->type(cc)->preoptyperesolve(cc, op);
}

void *PreOp::exprgen(CC *cc){
  return value->type(cc)->preopgen(cc, op, value);
}

void *PreOp::allocagen(CC *cc){
  graceFulExit(dbg, "Calling alloca on unsupported type");
  return 0;
}
