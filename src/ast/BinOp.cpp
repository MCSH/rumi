#include "BinOp.h"
#include "../base.h"

BinOp::BinOp(Expression *lhs, std::string op, Expression *rhs)
  : lhs(lhs)
  , op(op)
  , rhs(rhs)
{}

void BinOp::compile(CC *cc){
  lhs->compile(cc);
  rhs->compile(cc);

  if(!lhs->type(cc)->hasOp(cc, op, rhs)){
    cc->debug(NONE) << "Type doesn't support ops" << std::endl;
    exit(1);
  }
}

void BinOp::prepeare(CC *cc){
  lhs->prepeare(cc);
  rhs->prepeare(cc);
}

Type *BinOp::type(CC *cc){
  // TODO provide a correct type
  return lhs->type(cc);
}

void *BinOp::exprgen(CC *cc){
  return lhs->type(cc)->opgen(cc, lhs, op, rhs);
}
