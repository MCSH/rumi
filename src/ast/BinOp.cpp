#include "BinOp.h"
#include "../base.h"
#include "PointerType.h"
#include "ast.h"

BinOp::BinOp(Expression *lhs, std::string op, Expression *rhs)
  : lhs(lhs)
  , op(op)
  , rhs(rhs)
{}

void BinOp::compile(CC *cc){
  lhs->compile(cc);
  rhs->compile(cc);

  if(!lhs->type(cc)->hasOp(cc, op, rhs)){
    graceFulExit(dbg, "Type " + lhs->type(cc)->toString() +  "  doesn't support op " + op + " with " + rhs->type(cc)->toString());
  }
}

void BinOp::prepare(CC *cc){
  lhs->prepare(cc);
  rhs->prepare(cc);
}

Type *BinOp::type(CC *cc){
  return lhs->type(cc)->optyperesolve(cc, op, rhs);
}

void *BinOp::exprgen(CC *cc){
  return lhs->type(cc)->opgen(cc, lhs, op, rhs);
}

void *BinOp::allocagen(CC *cc){
  graceFulExit(dbg, "Calling alloca on unsupported type");
  return 0;
}

