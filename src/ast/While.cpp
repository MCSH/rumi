#include "While.h"
#include "../base.h"
#include "../LLContext.h"

While::While(Expression *condition, Statement *st)
  : condition(condition)
  , st(st)
{
}

void While::prepare(CC *cc){
  condition->prepare(cc);
  st->prepare(cc);
}

void While::compile(CC *cc){
  condition->compile(cc);
  st->compile(cc);
}

void While::codegen(CC *cc){
  auto f = cc->llc->f;
  llvm::BasicBlock *whileCondB = llvm::BasicBlock::Create(cc->llc->context, "whileCond", f),
    *whileBodyB = llvm::BasicBlock::Create(cc->llc->context, "whileBody", f),
    *whileContB = llvm::BasicBlock::Create(cc->llc->context, "whileCont", f);

  cc->llc->builder->CreateBr(whileCondB);

  cc->llc->builder->SetInsertPoint(whileCondB);
  // TODO clean this up
  auto cond = (llvm::Value*)condition->exprgen(cc);
  auto zeroConst = llvm::ConstantInt::get(llvm::IntegerType::getInt64Ty(cc->llc->context), 0, true);
  cond = cc->llc->builder->CreateICmpNE(cond, zeroConst);
  cc->llc->builder->CreateCondBr(cond, whileBodyB, whileContB);

  cc->llc->builder->SetInsertPoint(whileBodyB);
  st->codegen(cc);
  cc->llc->builder->CreateBr(whileCondB);

  cc->llc->builder->SetInsertPoint(whileContB);
}
