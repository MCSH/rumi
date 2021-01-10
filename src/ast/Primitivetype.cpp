#include "PrimitiveType.h"
#include "../base.h"
#include "../LLContext.h"
#include <ostream>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include "Expression.h"

PrimitiveType::PrimitiveType(TypeEnum key)
  : key(key)
{
}

void PrimitiveType::compile(CC *cc) {}
void PrimitiveType::prepare(CC *cc) {}

void *PrimitiveType::typegen(CC *cc){

  switch(key){
  case t_int:
  case t_s64:
    return llvm::IntegerType::get(cc->llc->context, 64);
  case t_unit:
    return llvm::Type::getVoidTy(cc->llc->context);
  }

  cc->debug(NONE) << "Not supporting this type in primitivetypes!" << std::endl;
  exit(1);
}

bool isInt(TypeEnum key){
  return key == t_int || key == t_u8 || key == t_u16 || key == t_u32 || key == t_u64 || key == t_s8 || key == t_s16 || key == t_s32 || key == t_s64;
}

Type* PrimitiveType::optyperesolve(CC *cc, std::string op, Expression *rhs){
  // TODO
  if((op == "+" || op == "-" || op == "*" || op == "/" || op == "%")
     && isInt(key)){
    Type *rt = rhs->type(cc);
    if(PrimitiveType *prt = dynamic_cast<PrimitiveType*>(rt)){
      if (isInt(prt->key))
        return this;
    }
  }

  return 0;
}

bool PrimitiveType::hasOp(CC *cc, std::string op, Expression *rhs){
  // TODO
  if((op == "+" || op == "-" || op == "*" || op == "/" || op == "%")
     && isInt(key)){
    Type *rt = rhs->type(cc);
    if(PrimitiveType *prt = dynamic_cast<PrimitiveType*>(rt)){
      if (isInt(prt->key))
        return true;
    }
  }

  return false;
}

void *PrimitiveType::opgen(CC *cc, Expression *lhs,  std::string op, Expression *rhs){
  Type *rt = rhs->type(cc);
  PrimitiveType *prt = dynamic_cast<PrimitiveType*>(rt);
  if(isInt(key) && isInt(prt->key)){
    if(op == "+"){
      return cc->llc->builder->CreateAdd((llvm::Value*)lhs->exprgen(cc), (llvm::Value*)rhs->exprgen(cc));
    }

    if(op == "*"){
      return cc->llc->builder->CreateMul((llvm::Value*)lhs->exprgen(cc), (llvm::Value*)rhs->exprgen(cc));
    }
  }

  cc->debug(NONE) << "INCOMPLETE IMPLEMENTATION FOR OPGEN PRIM"  << std::endl;
  exit(1);
  return 0;
}
