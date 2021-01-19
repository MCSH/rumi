#include "Cast.h"
#include "../base.h"

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
    cc->debug(NONE) << "Explicitly casting incompatible types" << std::endl;
    exit(1);
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
  cc->debug(NONE) << "Calling alloca on unsupported type" << std::endl;
  exit(1);
}
