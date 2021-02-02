#include "Indexing.h"

Indexing::Indexing(Expression *expr, Expression *index)
    : expr(expr), index(index) {}

void Indexing::compile(CC *cc){
  expr->compile(cc);
  index->compile(cc);

  // validate type matches
  if(!expr->type(cc)->hasIndex(cc, index)){
    graceFulExit(dbg, "Type " + expr->type(cc)->toString() +  "  doesn't support indexing of type " + index->type(cc)->toString());
  }
}

void Indexing::prepare(CC *cc){
  expr->prepare(cc);
  index->prepare(cc);
}

void *Indexing::exprgen(CC *cc){
  return expr->type(cc)->indexgen(cc, expr, index);
}

void *Indexing::allocagen(CC *cc){
  return expr->type(cc)->indexallocagen(cc, expr, index);
}

Type *Indexing::type(CC *cc){
  return expr->type(cc)->indextyperesolve(cc, index);
}
