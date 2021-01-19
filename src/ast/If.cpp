#include "If.h"
#include "../base.h"
#include "../LLContext.h"
#include "PrimitiveType.h"
#include "Cast.h"

If::If(Expression *condition, Statement *st1, Statement *st2)
  : condition(condition)
  , st1(st1)
  , st2(st2)
{}

void If::prepare(CC *cc){
  condition->prepare(cc);
  st1->prepare(cc);
  if(st2)st2->prepare(cc);
}

void If::compile(CC *cc){
  condition->compile(cc);
  st1->compile(cc);
  if(st2)st2->compile(cc);

  // check condition type
  auto booleanType = new PrimitiveType(t_bool);

  if(condition->type(cc)->compatible(cc, booleanType) != OK){
    condition = new Cast(condition, booleanType);
  }
}

void If::codegen(CC *cc){
  // TODO
  auto v = (llvm::Value*) condition->exprgen(cc);

  llvm::Function *f = cc->llc->f;

  llvm::BasicBlock *stTrueB = llvm::BasicBlock::Create(cc->llc->context, "ifT", f);
  llvm::BasicBlock *ifCont = llvm::BasicBlock::Create(cc->llc->context, "ifC", f);
  llvm::BasicBlock *stFalseB;
  if(st2) stFalseB = llvm::BasicBlock::Create(cc->llc->context, "ifF", f);
  else stFalseB = ifCont;

  cc->llc->builder->CreateCondBr(v, stTrueB, stFalseB);

  // generate true statement
  cc->llc->builder->SetInsertPoint(stTrueB);
  st1->codegen(cc);
  // TODO check ifbrok
  cc->llc->builder->CreateBr(ifCont);

  // generate false statement
  if(st2){
    cc->llc->builder->SetInsertPoint(stFalseB);
    st2->codegen(cc);
    // TODO check ifbrok
    cc->llc->builder->CreateBr(ifCont);
  }

  // go to cont
  cc->llc->builder->SetInsertPoint(ifCont);
}
