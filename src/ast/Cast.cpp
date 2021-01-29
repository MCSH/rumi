#include "Cast.h"
#include "../base.h"
#include "ast.h"

Cast::Cast(Expression *exp, Type *type_)
  : exp(exp)
  , type_(type_)
{}

void Cast::compile(CC *cc){
  exp->compile(cc);
  type_->compile(cc);

  // Ensure they are compatible
  auto compatibility = type_->compatible(cc, exp->type(cc)->baseType(cc));
  if(compatibility == INCOMPATIBLE){
    graceFulExit(dbg, "Explicitly casting incompatible types");
  }
}

void Cast::prepare(CC *cc){
  exp->prepare(cc);
  type_->prepare(cc);
}

Type* Cast::type(CC *cc){
  return type_;
}

void *Cast::exprgen(CC *cc){
  return type_->castgen(cc, exp);
}

void *Cast::allocagen(CC *cc){
  graceFulExit(dbg, "Calling alloca on unsupported type");
  return 0;
}
