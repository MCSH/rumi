#include "Address.h"
#include "PointerType.h"
#include "../base.h"
#include "ast.h"

// NOTE: Address should not rely on prepare / compile directly. The inner exp will always get their prepare/compile called, but address might not. (EX: Struct op gen)

Address::Address(Expression *exp)
  : exp(exp)
{}

void Address::compile(CC *cc){
  exp->compile(cc);
}

void Address::prepare(CC *cc){
  exp->prepare(cc);
}

void *Address::exprgen(CC *cc){
  return exp->allocagen(cc);
}

Type *Address::type(CC *cc){
  Type *innerType = exp->type(cc);
  return new PointerType(innerType);
}

void *Address::allocagen(CC *cc){
  graceFulExit(dbg, "Calling alloca on unsupported type");
  return 0;
}
