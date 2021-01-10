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
  case t_s8:
  case t_u8:
    return llvm::IntegerType::get(cc->llc->context, 8);
  case t_s16:
  case t_u16:
    return llvm::IntegerType::get(cc->llc->context, 16);
  case t_s32:
  case t_u32:
    return llvm::IntegerType::get(cc->llc->context, 32);
  case t_int:
  case t_s64:
  case t_u64:
    return llvm::IntegerType::get(cc->llc->context, 64);
  case t_unit:
    return llvm::Type::getVoidTy(cc->llc->context);

  case t_string:
    return llvm::PointerType::getUnqual(llvm::IntegerType::get(cc->llc->context, 8));
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
  // TODO take care of casting
  Type *rt = rhs->type(cc);
  PrimitiveType *prt = dynamic_cast<PrimitiveType*>(rt);
  if(isInt(key) && isInt(prt->key)){
    if(op == "+"){
      return cc->llc->builder->CreateAdd((llvm::Value*)lhs->exprgen(cc), (llvm::Value*)rhs->exprgen(cc));
    }

    if(op == "*"){
      return cc->llc->builder->CreateMul((llvm::Value*)lhs->exprgen(cc), (llvm::Value*)rhs->exprgen(cc));
    }

    if(op == "-"){
      return cc->llc->builder->CreateSub((llvm::Value*)lhs->exprgen(cc), (llvm::Value*)rhs->exprgen(cc));
    }
  }

  cc->debug(NONE) << "INCOMPLETE IMPLEMENTATION FOR OPGEN PRIM"  << std::endl;
  exit(1);
  return 0;
}

Compatibility PrimitiveType::compatible(Type *t){
  // TODO implement this.
  PrimitiveType *pt = dynamic_cast<PrimitiveType *>(t);
  if(!pt) return INCOMPATIBLE;

  if(key == pt->key)
    return OK;

  if(isInt(key) && isInt(pt->key)){
    if(key == t_u8 || key == t_s8){
      switch(pt->key){
      case t_u8:
      case t_int:
      case t_s8:
        return OK;
      default:
        return ExpCast;
      }
    }
    if(key == t_u16 || key == t_s16){
      switch(pt->key){
      case t_u8:
      case t_s8:
        return ImpCast;
      case t_u16:
      case t_s16:
      case t_int:
        return OK;
      default:
        return ExpCast;
      }
    }
    if(key == t_u32 || key == t_s32){
      switch(pt->key){
      case t_u8:
      case t_s8:
      case t_u16:
      case t_s16:
        return ImpCast;
      case t_u32:
      case t_s32:
      case t_int:
        return OK;
      default:
        return ExpCast;
      }
    }
    if(key == t_u64 || key == t_s64 || key == t_int){
      switch(pt->key){
      case t_u64:
      case t_s64:
      case t_int:
        return OK;
      default:
        return ImpCast;
      }
    }
  }

  return INCOMPATIBLE;
}

Expression *PrimitiveType::castFrom(Expression *e){
  // TODO
  return e;
}
