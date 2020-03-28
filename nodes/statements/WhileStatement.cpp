#include "WhileStatement.h"
#include "../../Context.h"


WhileStatement::~WhileStatement(){
    delete w;
    delete exp;
  }
void WhileStatement::codegen(Context *cc){
  // TODO

  llvm::Function *f = cc->builder->GetInsertBlock()->getParent();

  llvm::BasicBlock
    *condB = llvm::BasicBlock::Create(cc->context, "whilecond", f),
    *whileB = llvm::BasicBlock::Create(cc->context, "while"),
    *mergeB = llvm::BasicBlock::Create(cc->context, "whilecont");

  cc->builder->CreateBr(condB);

  // While cond
  this->cb.bblock = condB;
  cc->pushBlock(&this->cb);
  cc->builder->SetInsertPoint(condB);
  llvm::Value *cond = this->exp->exprGen(cc);
  cond = cc->builder->CreateICmpNE(cond, llvm::ConstantInt::get(llvm::Type::getInt64Ty(cc->context), 0, false), "whilecond"); // TODO improve?
  cc->builder->CreateCondBr(cond, whileB, mergeB);
  cc->popBlock();

  // While Body
  f->getBasicBlockList().push_back(whileB);
  this->bblock = whileB;
  cc->pushBlock(this);
  cc->builder->SetInsertPoint(whileB);
  this->w->codegen(cc);
  if (cc->builder->GetInsertPoint()
          ->getPrevNonDebugInstruction()
          ->getOpcode() != llvm::Instruction::Br) {
    cc->builder->CreateBr(condB);
  }
  cc->popBlock();

  // Cont
  f->getBasicBlockList().push_back(mergeB);
  cc->builder->SetInsertPoint(mergeB);
}

void WhileStatement::compile(Context *cc){
  auto ws = this; // TODO lazy
  cc->pushBlock(&this->cb);
  ws->exp->compile(cc);
  cc->popBlock();

  cc->pushBlock(this);
  ws->w->compile(cc);
  cc->popBlock();
  return;
}

