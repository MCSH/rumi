#include "String.h"
#include "../base.h"
#include "../LLContext.h"
#include "PrimitiveType.h"

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
  cc->debug(NONE) << "Calling alloca on unsupported type" << std::endl;
  exit(1);
}

void String::set(std::string key, void *value){
  if(key == "val"){
    val = std::string((char *) value);
    return;
  }

  // TODO error?
}

void String::add(std::string key, void *value){
  // TODO error?
}
