#include "MethodCall.h"
#include "PointerType.h"
#include "Struct.h"
#include "../base.h"
#include "../LLContext.h"
#include "Address.h"

void MethodCall::compile(CC *cc){
  // TODO
  exp->exp->compile(cc);

  s = dynamic_cast<StructType *>(exp->exp->type(cc)->baseType(cc));
  if(!s){
    cc->debug(NONE) << "Can't call method on non-structs" << std::endl;
    exit(1);
  }

  method = s->resolveMethod(cc, exp->id);
  if(!method){
    cc->debug(NONE) << "Can't find method " << exp->id << " in struct " << s->id << std::endl;
    exit(1);
  }

  // TODO add self to arguments
  auto base = exp->exp;
  if(PointerType *pt = dynamic_cast<PointerType *>(base->type(cc))){
    // TODO make sure its only one level deep 
  } else {
    base = new Address(base);
    base->prepare(cc);
    base->compile(cc);
  }

  args.insert(args.begin(), base);

  auto fname = s->id + "$" + exp->id;
  // TODO setup fcall
  fcall = new FCall();
  fcall->id = fname;
  fcall->args = args;

  fcall->prepare(cc);
  fcall->compile(cc);
}
void MethodCall::prepare(CC *cc){
  exp->exp->prepare(cc);
}
void* MethodCall::exprgen(CC *cc){
  return fcall->exprgen(cc);
}
Type* MethodCall::type(CC *cc){
  return method->f->returnType;
}
void* MethodCall::allocagen(CC *cc){
  return 0;
}

MethodCallStmt::MethodCallStmt(MethodCall *mc)
  : mc(mc)
{}

void MethodCallStmt::compile(CC *cc){
  mc->compile(cc);
}
void MethodCallStmt::prepare(CC *cc){
  mc->prepare(cc);
}
void MethodCallStmt::codegen(CC *cc){
  mc->exprgen(cc);
}
