#include "While.h"
#include "../base.h"
#include "../LLContext.h"
#include "PrimitiveType.h"
#include "Cast.h"

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

  // check condition type
  auto booleanType = new PrimitiveType(t_bool);

  if(condition->type(cc)->compatible(cc, booleanType) != OK){
    condition = new Cast(condition, booleanType);
  }
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
  cc->llc->builder->CreateCondBr(cond, whileBodyB, whileContB);

  cc->llc->builder->SetInsertPoint(whileBodyB);
  st->codegen(cc);
  cc->llc->builder->CreateBr(whileCondB);

  cc->llc->builder->SetInsertPoint(whileContB);
}

void While::set(std::string key, void *value){
  if(key == "condition"){
    condition = (Expression *) value;
    return;
  }

  if(key == "st"){
    st = (Statement *) value;
    return;
  }

  // TODO error?
}

void While::add(std::string key, void *value){
  // TODO error?
}
