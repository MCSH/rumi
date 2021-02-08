#include "PrimitiveType.h"
#include "../base.h"
#include "../LLContext.h"
#include <llvm/IR/Constants.h>
#include <ostream>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include "Cast.h"
#include "Expression.h"
#include "PointerType.h"
#include "Type.h"
#include "ast.h"

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
  case t_any:
    return llvm::IntegerType::get(cc->llc->context, 64);
  case t_unit:
    return llvm::Type::getVoidTy(cc->llc->context);
  case t_string:
    return llvm::PointerType::getUnqual(llvm::IntegerType::get(cc->llc->context, 8));
  case t_f32:
    return llvm::Type::getFloatTy(cc->llc->context);
  case t_f64:
    return llvm::Type::getDoubleTy(cc->llc->context);
  }

  graceFulExit(dbg, "Not supporting this type in primitive types!");
  return 0;
}

bool isFloat(TypeEnum key){
  return key == t_f32 || key == t_f64;
}

#include <iostream>

bool isBiggerOrEq(TypeEnum k1, TypeEnum k2){
  // only on integers
  if(k1 == t_u64) return true;
  if(k1 == t_s64) return true;
  if(k1 == t_int) return true;
  if(k1 == t_u32 || k1 == t_s32){
    if(k2 == t_u64 || k2 == t_s64 || k2 == t_int) return false;
    return true;
  }
  if(k1 == t_u16 || k1 == t_s16){
    if(k2 == t_u16 || k2 == t_s16 || k2 == t_s8 || k2 == t_u8 || k2 == t_bool) return true;
    return false;
  }
  if(k1 == t_u8 || k1 == t_s8){
    if(k2 == t_s8 || k2 == t_u8 || k2 == t_bool) return true;
    return false;
  }
  if(k1 == t_bool) return k2 == t_bool;
  std::cout << "isBiggerOrEq called on types other than int" << std::endl;
  exit(1);
  return false;
}

bool isInt(TypeEnum key){
  return key == t_int || key == t_u8 || key == t_u16 || key == t_u32 || key == t_u64 || key == t_s8 || key == t_s16 || key == t_s32 || key == t_s64 || key == t_any || key == t_bool;
}

bool isSigned(TypeEnum key){
  switch(key){
  case t_int:
  case t_s64:
  case t_s32:
  case t_s16:
  case t_s8:
  case t_f32:
  case t_f64:
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
  if((op == "+" || op == "-" || op == "*" || op == "/" || op == "%" || op == "^" || op == "&" || op == "|" || op == ">>" || op == "<<")
     && isInt(key)){
    Type *rt = rhs->type(cc);
    if(PrimitiveType *prt = dynamic_cast<PrimitiveType*>(rt)){
      if(isInt(prt->key)){
        if(isBiggerOrEq(key, prt->key))
          return this;
        else
          return rt;
      }
      if(isFloat(prt->key))
        return prt;
    }
  }
  if(op == "+" || op == "-" || op == "*" || op == "/" || op == "%"){
    if(isFloat(key)){
      Type *rt = rhs->type(cc);
      if(PrimitiveType *prt = dynamic_cast<PrimitiveType*>(rt)){
        if(key == prt->key) return this;
        if(prt->key == t_f64) return prt;
        if(isFloat(prt->key) || isInt(prt->key))
          return this;
      }
    }
  }

  if(op == "==" || op =="!=" || op=="<" || op=="<=" || op==">" || op==">=" || op == "&&" || op == "||"){
    return new PrimitiveType(t_bool);
  }

  return 0;
}

bool PrimitiveType::hasOp(CC *cc, std::string op, Expression *rhs){
  // TODO

  if(op == "^" || op == "|" || op == "&" || op == ">>" || op == "<<"){
    if(isInt(key)){
      Type *rt = rhs->type(cc);
      if(PrimitiveType *prt = dynamic_cast<PrimitiveType*>(rt)){
        if(isInt(prt->key)) return true;
      }
    }
  }
  
  if(op == "&&" || op == "||"){
    PrimitiveType booleanType(t_bool);
    if(compatible(cc, &booleanType))
      if(rhs->type(cc)->compatible(cc, &booleanType))
        return true;
  }

  if(op == ">" || op == ">=" || op == "<" || op == "<="){
    if(compatible(cc, rhs->type(cc)) != INCOMPATIBLE)
      if(isInt(key)){
        return true;
      }
  }
  if((op == "==" || op == "!=") && key != t_string){
    if(compatible(cc, rhs->type(cc)) != INCOMPATIBLE){
      // TODO float vs int comparison might be problematic... handle it somehow
      if(isInt(key)|| isFloat(key))
        return true;
    }
  }
  if(op == "+" || op == "-" || op == "*" || op == "/" || op == "%"){
    if (isInt(key) || isFloat(key)) {
      Type *rt = rhs->type(cc);
      if (PrimitiveType *prt = dynamic_cast<PrimitiveType *>(rt)) {
        if (isInt(prt->key) || isFloat(prt->key))
          return true;
      }
    }
  }

  return false;
}

void *PrimitiveType::opgen(CC *cc, Expression *lhs,  std::string op, Expression *rhs){
  // TODO take care of casting
  if(op == ">>"){
    if(isInt(key)){
      Type *rt = rhs->type(cc);
      if(PrimitiveType *prt = dynamic_cast<PrimitiveType*>(rt)){
        if(isInt(prt->key))
          return cc->llc->builder->CreateAShr((llvm::Value *)lhs->exprgen(cc), (llvm::Value *)rhs->exprgen(cc));
      }
    }
  }
  if(op == "<<"){
    if(isInt(key)){
      Type *rt = rhs->type(cc);
      if(PrimitiveType *prt = dynamic_cast<PrimitiveType*>(rt)){
        if(isInt(prt->key))
          return cc->llc->builder->CreateShl((llvm::Value *)lhs->exprgen(cc), (llvm::Value *)rhs->exprgen(cc));
      }
    }
  }
  if(op == "^"){
    if(isInt(key)){
      Type *rt = rhs->type(cc);
      if(PrimitiveType *prt = dynamic_cast<PrimitiveType*>(rt)){
        if(isInt(prt->key))
          return cc->llc->builder->CreateXor((llvm::Value *)lhs->exprgen(cc), (llvm::Value *)rhs->exprgen(cc));
      }
    }
  }
  if( op == "|"){
    if(isInt(key)){
      Type *rt = rhs->type(cc);
      if(PrimitiveType *prt = dynamic_cast<PrimitiveType*>(rt)){
        if(isInt(prt->key))
          return cc->llc->builder->CreateOr((llvm::Value *)lhs->exprgen(cc), (llvm::Value *)rhs->exprgen(cc));
      }
    }
  }
  if(op == "&"){
    if(isInt(key)){
      Type *rt = rhs->type(cc);
      if(PrimitiveType *prt = dynamic_cast<PrimitiveType*>(rt)){
        if(isInt(prt->key))
          return cc->llc->builder->CreateAnd((llvm::Value *)lhs->exprgen(cc), (llvm::Value *)rhs->exprgen(cc));
      }
    }
  }

  if(op == ">"){
    if(isInt(key)){
      if(isSigned(key))
        return cc->llc->builder->CreateICmpSGT((llvm::Value*) lhs->exprgen(cc), (llvm::Value*) rhs->exprgen(cc));
      else
        return cc->llc->builder->CreateICmpUGT((llvm::Value*) lhs->exprgen(cc), (llvm::Value*) rhs->exprgen(cc));
    }
  }
  if(op == ">="){
    if(isInt(key)){
      if(isSigned(key))
        return cc->llc->builder->CreateICmpSGE((llvm::Value*) lhs->exprgen(cc), (llvm::Value*) rhs->exprgen(cc));
      else
        return cc->llc->builder->CreateICmpUGE((llvm::Value*) lhs->exprgen(cc), (llvm::Value*) rhs->exprgen(cc));
    }
  }
  if(op == "<"){
    if(isInt(key)){
      if(isSigned(key))
        return cc->llc->builder->CreateICmpSLT((llvm::Value*) lhs->exprgen(cc), (llvm::Value*) rhs->exprgen(cc));
      else
        return cc->llc->builder->CreateICmpULT((llvm::Value*) lhs->exprgen(cc), (llvm::Value*) rhs->exprgen(cc));
    }
  }
  if(op == "<="){
    if(isInt(key)){
      if(isSigned(key))
        return cc->llc->builder->CreateICmpSLE((llvm::Value*) lhs->exprgen(cc), (llvm::Value*) rhs->exprgen(cc));
      else
        return cc->llc->builder->CreateICmpULE((llvm::Value*) lhs->exprgen(cc), (llvm::Value*) rhs->exprgen(cc));
    }
  }
  if(op == "!="){
    if(isInt(key)){
      return cc->llc->builder->CreateICmpNE((llvm::Value*) lhs->exprgen(cc), (llvm::Value*) rhs->exprgen(cc));
    } else if(isFloat(key)){
      return cc->llc->builder->CreateFCmpUNE((llvm::Value*) lhs->exprgen(cc), (llvm::Value*) rhs->exprgen(cc));
    }
  }

  if(op == "=="){
    if(isInt(key)){
      return cc->llc->builder->CreateICmpEQ((llvm::Value*) lhs->exprgen(cc), (llvm::Value*) rhs->exprgen(cc));
    } else if(isFloat(key)){
      return cc->llc->builder->CreateFCmpUEQ((llvm::Value*) lhs->exprgen(cc), (llvm::Value*) rhs->exprgen(cc));
    } else {
      graceFulExit(dbg, "== Not supported for this type");
    }
  }

  if(op == "&&"){
    PrimitiveType booleanType(t_bool);
    lhs = new Cast(lhs, &booleanType);
    rhs = new Cast(rhs, &booleanType);
    lhs->prepare(cc);
    lhs->compile(cc);
    rhs->prepare(cc);
    rhs->compile(cc);

    auto out = cc->llc->builder->CreateAlloca((llvm::Type*)booleanType.typegen(cc), 0, "andtmpval");

    auto f = cc->llc->f;

    llvm::BasicBlock *condB = llvm::BasicBlock::Create(cc->llc->context, "andCond", f);
    llvm::BasicBlock *falseB = llvm::BasicBlock::Create(cc->llc->context, "andFalse", f);
    llvm::BasicBlock *trueB = llvm::BasicBlock::Create(cc->llc->context, "andTrue", f);
    llvm::BasicBlock *contB = llvm::BasicBlock::Create(cc->llc->context, "andContB", f);

    cc->llc->builder->CreateCondBr((llvm::Value*)lhs->exprgen(cc), condB, falseB);
    cc->llc->builder->SetInsertPoint(condB);
    cc->llc->builder->CreateCondBr((llvm::Value*)rhs->exprgen(cc), trueB, falseB);
    cc->llc->builder->SetInsertPoint(falseB);
    auto falseV = llvm::ConstantInt::get(llvm::IntegerType::getInt1Ty(cc->llc->context), 0, true);
    cc->llc->builder->CreateStore(falseV, out);
    cc->llc->builder->CreateBr(contB);
    cc->llc->builder->SetInsertPoint(trueB);
    auto trueV = llvm::ConstantInt::get(llvm::IntegerType::getInt1Ty(cc->llc->context), 1, true);
    cc->llc->builder->CreateStore(trueV, out);
    cc->llc->builder->CreateBr(contB);
    cc->llc->builder->SetInsertPoint(contB);
    return cc->llc->builder->CreateLoad(out);
  }

  if(op == "||"){
    PrimitiveType booleanType(t_bool);
    lhs = new Cast(lhs, &booleanType);
    rhs = new Cast(rhs, &booleanType);
    lhs->prepare(cc);
    lhs->compile(cc);
    rhs->prepare(cc);
    rhs->compile(cc);

    auto out = cc->llc->builder->CreateAlloca((llvm::Type*)booleanType.typegen(cc), 0, "ortmpval");

    auto f = cc->llc->f;

    llvm::BasicBlock *condB = llvm::BasicBlock::Create(cc->llc->context, "orCond", f);
    llvm::BasicBlock *falseB = llvm::BasicBlock::Create(cc->llc->context, "orFalse", f);
    llvm::BasicBlock *trueB = llvm::BasicBlock::Create(cc->llc->context, "orTrue", f);
    llvm::BasicBlock *contB = llvm::BasicBlock::Create(cc->llc->context, "orContB", f);

    cc->llc->builder->CreateCondBr((llvm::Value*)lhs->exprgen(cc), trueB, condB);
    cc->llc->builder->SetInsertPoint(condB);
    cc->llc->builder->CreateCondBr((llvm::Value*)rhs->exprgen(cc), trueB, falseB);
    cc->llc->builder->SetInsertPoint(falseB);
    auto falseV = llvm::ConstantInt::get(llvm::IntegerType::getInt1Ty(cc->llc->context), 0, true);
    cc->llc->builder->CreateStore(falseV, out);
    cc->llc->builder->CreateBr(contB);
    cc->llc->builder->SetInsertPoint(trueB);
    auto trueV = llvm::ConstantInt::get(llvm::IntegerType::getInt1Ty(cc->llc->context), 1, true);
    cc->llc->builder->CreateStore(trueV, out);
    cc->llc->builder->CreateBr(contB);
    cc->llc->builder->SetInsertPoint(contB);
    return cc->llc->builder->CreateLoad(out);
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

  if(op == "+" || op == "*" || op == "-" || op == "/" || op == "%"){
    // TODO validate this
    // at least one isn't a float, are we 32?
    bool is32 = (key != t_f64) && (prt->key != t_f64);
    auto t = llvm::Type::getDoubleTy(cc->llc->context);
    if(is32){
      t = llvm::Type::getFloatTy(cc->llc->context);
    }
    auto l = (llvm::Value*)lhs->exprgen(cc);
    auto r = (llvm::Value*)rhs->exprgen(cc);
    if(isInt(key) && isSigned(key)) l = cc->llc->builder->CreateSIToFP(l, t);
    if(isInt(key) && !isSigned(key)) l = cc->llc->builder->CreateUIToFP(l, t);
    if(isInt(prt->key) && isSigned(prt->key)) l = cc->llc->builder->CreateSIToFP(r, t);
    if(isInt(prt->key) && !isSigned(prt->key)) l = cc->llc->builder->CreateUIToFP(r, t);
    // handle f32 and f64 conversion...
    if(!is32 && key == t_f32) l = cc->llc->builder->CreateFPCast(l, t);
    if(!is32 && prt->key == t_f32) r = cc->llc->builder->CreateFPCast(r, t);
    if(op == "+") return cc->llc->builder->CreateFAdd(l, r);
    if(op == "-") return cc->llc->builder->CreateFSub(l, r);
    if(op == "*") return cc->llc->builder->CreateFMul(l, r);
    if(op == "/") return cc->llc->builder->CreateFDiv(l, r);
    if(op == "%") return cc->llc->builder->CreateFRem(l, r);
  }

  graceFulExit(dbg, "INCOMPLETE IMPLEMENTATION FOR OPGEN PRIMITIVE");
  return 0;
}

Compatibility PrimitiveType::compatible(CC *cc, Type *t){
  if(key == t_string){
    // Also accept *u8
    if(PointerType *pt = dynamic_cast<PointerType *>(t)){
      if(PrimitiveType *p = dynamic_cast<PrimitiveType*>(pt->innerType)){
        if(p->key == t_u8) return OK;
      }
    }
  }

  if(key == t_any)
    return OK;

  // TODO implement this.
  PrimitiveType *pt = dynamic_cast<PrimitiveType *>(t);
  if(!pt){
    if(PointerType *pt = dynamic_cast<PointerType *>(t)){
      if(key == t_bool){
        return ImpCast;
      }
    }
    return INCOMPATIBLE; 
  }

  if(key == pt->key)
    return OK;


  if(pt->key == t_any)
    return OK;

  // float to float
  if(isFloat(key) && isFloat(pt->key)){
    if(key == t_f64){
      return ImpCast; // f32 -> f64
    }
    if(key == t_f32){
      return ExpCast; // f64 -> f32
    }
  }

  // int to float
  if(isFloat(key) && isInt(pt->key)){
    return ImpCast;
  }

  // float to int
  if(isInt(key) && isFloat(pt->key)){
    return ExpCast;
  }

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
  auto t = e->type(cc)->baseType(cc);

  if(key == t_string){
    if(PointerType *pt = dynamic_cast<PointerType *>(t)){
      if(PrimitiveType *p = dynamic_cast<PrimitiveType*>(pt->innerType)){
        if(p->key == t_u8) return e->exprgen(cc);
      }
    }
  }

  if(key == t_any)
    return e->exprgen(cc);

  PrimitiveType *pt = dynamic_cast<PrimitiveType *>(t);
  if(!pt){
    if(PointerType *pt = dynamic_cast<PointerType *>(t)){
      if(key == t_bool){
        auto i64t = llvm::Type::getInt64Ty(cc->llc->context);
        return cc->llc->builder->CreateICmpNE(
            cc->llc->builder->CreatePtrToInt((llvm::Value *)e->exprgen(cc),
                                             i64t),
            llvm::ConstantInt::get(i64t, 0));
      }
    }
    return 0;
  }

  if(key == pt->key)
    return e->exprgen(cc);

  if(pt->key == t_any){
    return e->exprgen(cc);
  }

  if(isFloat(key) && isFloat(pt->key)){
    return cc->llc->builder->CreateFPCast((llvm::Value*)e->exprgen(cc), (llvm::Type *)typegen(cc));
  }

  if(isFloat(key) && isInt(pt->key)){
    if(isSigned(pt->key))
      return cc->llc->builder->CreateSIToFP((llvm::Value *)e->exprgen(cc), (llvm::Type *)typegen(cc));
    return cc->llc->builder->CreateUIToFP((llvm::Value *)e->exprgen(cc), (llvm::Type *)typegen(cc));
  }

  if(isInt(key) && isFloat(pt->key)){
    if(isSigned(key))
      return cc->llc->builder->CreateFPToSI((llvm::Value*)e->exprgen(cc), (llvm::Type*)typegen(cc));
    return cc->llc->builder->CreateFPToUI((llvm::Value*)e->exprgen(cc), (llvm::Type*)typegen(cc));
  }

  if(!isInt(key) || !isInt(pt->key)){
    graceFulExit(dbg, "Calling cast on non-integer types");
  }

  if(key == t_bool){
    auto zeroConst = llvm::ConstantInt::get((llvm::Type*)pt->typegen(cc), 0, true);
    return cc->llc->builder->CreateICmpNE((llvm::Value*)e->exprgen(cc), zeroConst);
  }

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

  if(op == "~"){
    return isInt(key);
  }

  return false;
}

Type *PrimitiveType::preoptyperesolve(CC *cc, std::string op){
  if(op == "!"){
    if(key == t_bool)
      return this;
  }

  if(op == "~") return this;

  return 0;
}

void *PrimitiveType::preopgen(CC *cc, std::string op, Expression *value){
  if(op == "!"){
    if(key == t_bool){
      return cc->llc->builder->CreateICmpEQ((llvm::Value *)value->exprgen(cc),
                                            llvm::ConstantInt::get((llvm::Type *)typegen(cc), 0, false));
    }
  }

  if(op == "~"){
    auto negOne = llvm::ConstantInt::get((llvm::Type *)typegen(cc), -1, true);
    return cc->llc->builder->CreateXor((llvm::Value*) value->exprgen(cc), negOne);
  }

  return 0;
}

std::string PrimitiveType::toString(){
  return typeEnumToString(key);
}


void PrimitiveType::initgen(CC *cc, Expression *alloca){
  // Do nothing
}

bool PrimitiveType::hasIndex(CC *cc, Expression *index){
  return 0;
}

void *PrimitiveType::indexgen(CC *cc, Expression *expr, Expression *index){
  return 0;
}

Type *PrimitiveType::indextyperesolve(CC *cc, Expression *index){
  return 0;
}

void *PrimitiveType::indexallocagen(CC *cc, Expression *expr, Expression *index){
  return 0;
}
