#include "PointerType.h"
#include "Type.h"
#include "../LLContext.h"

PointerType::PointerType(Type *innerType)
  : innerType(innerType)    
{}

void PointerType::compile(CC *cc){
  innerType->compile(cc);
}

void PointerType::prepare(CC *cc){
  innerType->prepare(cc);
}

void *PointerType::typegen(CC *cc){
  return llvm::PointerType::getUnqual((llvm::Type *)innerType->typegen(cc));
}

bool PointerType::hasOp(CC *cc, std::string op, Expression *rhs){
  // TODO
  return 0;
}
void *PointerType::opgen(CC *cc, Expression *lhs, std::string op, Expression *rhs){
  // TODO
  return 0;
}
Type *PointerType::optyperesolve(CC *cc, std::string op, Expression *rhs){
  // TODO
  return 0;
}
Compatibility PointerType::compatible(Type *t){
  // TODO
  return INCOMPATIBLE;
}
void *PointerType::castgen(CC *cc, Expression *e){
  // TODO
  return 0;
}
