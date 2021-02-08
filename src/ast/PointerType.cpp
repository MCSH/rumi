#include "PointerType.h"
#include "PrimitiveType.h"
#include "Struct.h"
#include "Type.h"
#include "Expression.h"
#include "PrimitiveType.h"
#include "../base.h"
#include "../LLContext.h"
#include "PtrValue.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>

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
  Type *rhst = rhs->type(cc)->baseType(cc);
  if(PrimitiveType * pt = dynamic_cast<PrimitiveType *>(rhst)){
    if(isInt(pt->key) && (op == "+" || op == "-")){
      return true;
    }
  }
  if(StructType *st = dynamic_cast<StructType*>(innerType->baseType(cc))){
    return innerType->hasOp(cc, op, rhs);
  }
  return false;
}

void *PointerType::opgen(CC *cc, Expression *lhs, std::string op, Expression *rhs){
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
  if(StructType *st = dynamic_cast<StructType*>(innerType->baseType(cc))){
    return innerType->opgen(cc, new PtrValue(lhs), op, rhs);
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
  if(StructType *st = dynamic_cast<StructType*>(innerType->baseType(cc))){
    return innerType->optyperesolve(cc, op, rhs);
  }
  return 0;
}

Compatibility PointerType::compatible(CC *cc, Type *t){

  // Speical case, u8 from string
  if(PrimitiveType *pt = dynamic_cast<PrimitiveType *>(innerType)){
    if(pt->key == t_any) return OK;
    if(pt->key == t_u8)  
      if(PrimitiveType *opt = dynamic_cast<PrimitiveType*>(t))
        if(opt->key == t_string) return OK;
  }
  
  // TODO
  if(PointerType *pt = dynamic_cast<PointerType*>(t)){
    return OK;
  }

  // TODO improve
  if(PrimitiveType *pt = dynamic_cast<PrimitiveType*>(t)){
    return ImpCast;
  }
  
  return INCOMPATIBLE;
}
void *PointerType::castgen(CC *cc, Expression *e){
  // TODO

  // Speical case, u8 from string
  if(PrimitiveType *pt = dynamic_cast<PrimitiveType *>(innerType)){
    if(pt->key == t_any) return e->exprgen(cc);
    if(pt->key == t_u8)  
      if(PrimitiveType *opt = dynamic_cast<PrimitiveType*>(e->type(cc)->baseType(cc)))
        if(opt->key == t_string) return e->exprgen(cc);
  }

  // TODO improve
  if(PrimitiveType *pt = dynamic_cast<PrimitiveType*>(e->type(cc)->baseType(cc))){
    auto exp = (llvm::Value*)e->exprgen(cc);
    return cc->llc->builder->CreateIntToPtr(exp, (llvm::Type *)typegen(cc));
  }
  
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
  if(op == "!") return true;
  return false;
}

Type *PointerType::preoptyperesolve(CC *cc, std::string op){
  if(op == "!") return new PrimitiveType(t_bool);
  return 0;
}

void *PointerType::preopgen(CC *cc, std::string op, Expression *value) {
  if (op == "!")
    return cc->llc->builder->CreateICmpEQ(
        (llvm::Value *)value->exprgen(cc),
        llvm::ConstantPointerNull::get((llvm::PointerType *)typegen(cc)));
  return 0;
}

std::string PointerType::toString(){
  return "*" + innerType->toString();
}

void PointerType::initgen(CC *cc, Expression *alloca){
  // Do nothing
}

bool PointerType::hasIndex(CC *cc, Expression *index){
  Type *t = index->type(cc)->baseType(cc);
  if(PrimitiveType * pt = dynamic_cast<PrimitiveType *>(t)){
    if(isInt(pt->key))
      return true;
  }
  return false;
}

void *PointerType::indexgen(CC *cc, Expression *expr, Expression *index){
  return cc->llc->builder->CreateLoad((llvm::Value *)indexallocagen(cc, expr, index));
}

Type *PointerType::indextyperesolve(CC *cc, Expression *index){
  return innerType;
}

void *PointerType::indexallocagen(CC *cc, Expression *expr, Expression *index){
  auto v = (llvm::Value *)index->exprgen(cc); // int
  auto p = (llvm::Value *)expr->exprgen(cc); // ptr
  //auto p = (llvm::Value *)expr->allocagen(cc); // ptr

  std::vector<llvm::Value *> ind;
  ind.push_back(v);

  return cc->llc->builder->CreateInBoundsGEP(p, ind);
}
