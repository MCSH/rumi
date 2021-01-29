#include "String.h"
#include "../base.h"
#include "../LLContext.h"
#include "PrimitiveType.h"
#include "ast.h"

String::String(std::string val)
  : val(val)
{}

void String::compile(CC *cc){}
void String::prepare(CC *cc){}

void *String::exprgen(CC *cc){
  return cc->llc->builder->CreateGlobalString(val);
}

Type *String::type(CC *cc){
  return new PrimitiveType(t_string);
}

void *String::allocagen(CC *cc){
  graceFulExit(dbg, "Calling alloca on unsupported type");
  return 0;
}
