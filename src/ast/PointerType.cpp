#include "PointerType.h"
#include "PrimitiveType.h"
#include "Type.h"
#include "Expression.h"
#include "PrimitiveType.h"
#include "../base.h"
#include "../LLContext.h"
#include "PtrValue.h"

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
  Type *rhst = rhs->type(cc);
  if(PrimitiveType * pt = dynamic_cast<PrimitiveType *>(rhst)){
    if(isInt(pt->key) && (op == "+" || op == "-")){
      return true;
    }
  }
  return false;
}

void *PointerType::opgen(CC *cc, Expression *lhs, std::string op, Expression *rhs){
  // TODO
  Type *rhst = rhs->type(cc);
  if(PrimitiveType * pt = dynamic_cast<PrimitiveType *>(rhst)){
    if(isInt(pt->key) && (op == "+" || op == "-")){
      auto v = (llvm::Value*)rhs->exprgen(cc); // int
      auto p = (llvm::Value*)lhs->exprgen(cc); // ptr

      std::vector<llvm::Value *> ind;
      ind.push_back(v);

      return cc->llc->builder->CreateInBoundsGEP(p, ind);
    }
  }
  return 0;
}

Type *PointerType::optyperesolve(CC *cc, std::string op, Expression *rhs){
  Type *rhst = rhs->type(cc);
  if(PrimitiveType * pt = dynamic_cast<PrimitiveType *>(rhst)){
    if(isInt(pt->key) && (op == "+" || op == "-")){
      return this;
    }
  }
  return 0;
}

Compatibility PointerType::compatible(CC *cc, Type *t){
  // TODO
  if(PointerType *pt = dynamic_cast<PointerType*>(t)){
    return OK;
  }
  return INCOMPATIBLE;
}
void *PointerType::castgen(CC *cc, Expression *e){
  // TODO
  return e->exprgen(cc);
}

// TODO check to be only one layer deep

bool PointerType::hasMem(CC *cc, Expression *exp, std::string id){
  PtrValue pe(exp);
  return innerType->hasMem(cc, &pe, id);
}

void *PointerType::memgen(CC *cc, Expression *exp, std::string id){
  PtrValue pe(exp);
  return innerType->memgen(cc, &pe, id);
}

void *PointerType::memalloca(CC *cc, Expression *exp, std::string id){
  PtrValue pe(exp);
  return innerType->memalloca(cc, &pe, id);
}

Type *PointerType::memtyperesolve(CC *cc, Expression *exp, std::string id){
  PtrValue pe(exp);
  return innerType->memtyperesolve(cc, &pe, id);
}


bool PointerType::hasPreOp(CC *cc, std::string op){
  return false;
}

Type *PointerType::preoptyperesolve(CC *cc, std::string op){
  return 0;
}

void *PointerType::preopgen(CC *cc, std::string op, Expression *value){
  return 0;
}
