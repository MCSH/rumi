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
  case t_bool:
    return llvm::IntegerType::get(cc->llc->context, 1);
  case t_s8:
  case t_u8:
  case t_any:
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
  return key == t_int || key == t_u8 || key == t_u16 || key == t_u32 || key == t_u64 || key == t_s8 || key == t_s16 || key == t_s32 || key == t_s64 || key == t_any || key == t_bool;
}

int sizeInt(TypeEnum key){
  switch(key){
  case t_int:
  case t_s64:
  case t_u64:
    return 64;
  case t_u32:
  case t_s32:
    return 32;
  case t_u16:
  case t_s16:
    return 16;
  case t_u8:
  case t_s8:
  case t_any:
    return 8;
  case t_bool:
    return 1;
  default:
    return -1;
  }
}

#include <iostream>

bool isSigned(TypeEnum key){
  switch(key){
  case t_int:
  case t_s64:
  case t_s32:
  case t_s16:
  case t_s8:
    return true;
  case t_any:
  case t_u64:
  case t_u32:
  case t_u16:
  case t_u8:
  case t_bool:
    return false;
  default:
    std::cout << "Calling isSigned on non-signed types" << std::endl;
    exit(1);
  }
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

  if(op == "=="){
    return new PrimitiveType(t_bool);
  }

  return 0;
}

bool PrimitiveType::hasOp(CC *cc, std::string op, Expression *rhs){
  // TODO
  if(op == "==" || key != t_string)
    return true;
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

  if(op == "=="){
    if(isInt(key)){
      return cc->llc->builder->CreateICmpEQ((llvm::Value*) lhs->exprgen(cc), (llvm::Value*) rhs->exprgen(cc));
    } else {
      // TODO assuming float
      cc->debug(NONE) << " == Not supported for floating points" << std::endl;
      exit(1);
    }
  }
  
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

    if(op == "/"){
      if(isSigned(key))
        return cc->llc->builder->CreateSDiv((llvm::Value*)lhs->exprgen(cc), (llvm::Value*)rhs->exprgen(cc));
      else
        return cc->llc->builder->CreateUDiv((llvm::Value*)lhs->exprgen(cc), (llvm::Value*)rhs->exprgen(cc));
    }

    if(op == "%"){
      if(isSigned(key))
        return cc->llc->builder->CreateSRem((llvm::Value*)lhs->exprgen(cc), (llvm::Value*)rhs->exprgen(cc));
      else
        return cc->llc->builder->CreateURem((llvm::Value*)lhs->exprgen(cc), (llvm::Value*)rhs->exprgen(cc));
    }
  }

  cc->debug(NONE) << "INCOMPLETE IMPLEMENTATION FOR OPGEN PRIM"  << std::endl;
  exit(1);
  return 0;
}

Compatibility PrimitiveType::compatible(CC *cc, Type *t){
  // TODO implement this.
  PrimitiveType *pt = dynamic_cast<PrimitiveType *>(t);
  if(!pt) return INCOMPATIBLE;

  if(key == pt->key)
    return OK;

  if(isInt(key) && isInt(pt->key)){
    if(key == t_bool){
      switch(pt->key){
      case t_bool:
        return OK;
      default:
        return ExpCast;
      }
    }
    if(key == t_u8 || key == t_s8){
      switch(pt->key){
      case t_int:
      case t_any:
      case t_bool:
        return ImpCast;
      case t_u8:
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
      case t_int:
      case t_any:
      case t_bool:
        return ImpCast;
      case t_u16:
      case t_s16:
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
      case t_int:
      case t_any:
      case t_bool:
        return ImpCast;
      case t_u32:
      case t_s32:
        return OK;
      default:
        return ExpCast;
      }
    }
    if(key == t_u64 || key == t_s64 || key == t_int){
      switch(pt->key){
      case t_u64:
      case t_s64:
        return OK;
      default:
        return ImpCast;
      }
    }
  }

  return INCOMPATIBLE;
}

void *PrimitiveType::castgen(CC *cc, Expression *e){
  PrimitiveType *pt = dynamic_cast<PrimitiveType *>(e->type(cc));
  if(!pt) return 0;
  if(key == pt->key)
    return e->exprgen(cc);

  if(!isInt(key) || !isInt(pt->key)){
    std::cout << "Calling cast on non-integer types" << std::endl;
    exit(1);
  }

  std::cout << "Cast integer was invoked" << std::endl;
  return cc->llc->builder->CreateIntCast((llvm::Value*)e->exprgen(cc), (llvm::Type *)typegen(cc), isSigned(pt->key));
}

bool PrimitiveType::hasMem(CC *cc, Expression *exp, std::string id){
  return false;
}

void *PrimitiveType::memgen(CC *cc, Expression *exp, std::string id){
  return 0;
}

void *PrimitiveType::memalloca(CC *cc, Expression *exp, std::string id){
  return 0;
}

Type *PrimitiveType::memtyperesolve(CC *cc, Expression *exp, std::string id){
  return 0;
}

bool PrimitiveType::hasPreOp(CC *cc, std::string op){
  if(op == "!"){
    return key == t_bool;
  }

  return false;
}

Type *PrimitiveType::preoptyperesolve(CC *cc, std::string op){
  if(op == "!"){
    if(key == t_bool)
      return this;
  }

  return 0;
}

void *PrimitiveType::preopgen(CC *cc, std::string op, Expression *value){
  if(op == "!"){
    if(key == t_bool){
      return cc->llc->builder->CreateNot((llvm::Value *)value->exprgen(cc));
    }
  }

  return 0;
}
