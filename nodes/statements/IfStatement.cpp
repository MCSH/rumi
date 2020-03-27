#include "IfStatement.h"
#include <llvm/IR/BasicBlock.h>
#include "../../Context.h"

IfStatement::~IfStatement() {
  delete i;
  if (e)
    delete e;
  delete exp;
}
void IfStatement::codegen(Context *cc){
  llvm::Function *f = cc->builder->GetInsertBlock()->getParent();

  llvm::BasicBlock *ifB = llvm::BasicBlock::Create(cc->context, "if", f),
    *elseB,
    *mergeB = llvm::BasicBlock::Create(cc->context, "ifcont");

  if(this->e)
    elseB = llvm::BasicBlock::Create(cc->context, "else");
  else
    elseB = mergeB;

  llvm::Value *cond = this->exp->exprGen(cc);

  cond = cc->builder->CreateICmpNE(cond, llvm::ConstantInt::get(llvm::Type::getInt64Ty(cc->context), 0, false), // TODO, maybe improve?
    "ifcond");

  cc->builder->CreateCondBr(cond, ifB, elseB);

  // If Body
  cc->blocks.push_back(getIB(ifB));
  cc->builder->SetInsertPoint(ifB);
  this->i->codegen(cc);
  printf("lineno = %d\n", lineno);
  // TODO apply same mechanism in while, else, etc
  if (cc->builder->GetInsertBlock()->empty() ||
      cc->builder->GetInsertPoint()
              ->getPrevNonDebugInstruction()
              ->getOpcode() != llvm::Instruction::Br) {
    cc->builder->CreateBr(mergeB);
  }
  cc->blocks.pop_back();

  // Else
  if(this->e){
    cc->blocks.push_back(getEB(elseB));
    f->getBasicBlockList().push_back(elseB);
    cc->builder->SetInsertPoint(elseB);
    this->e->codegen(cc);
    if (cc->builder->GetInsertPoint()
            ->getPrevNonDebugInstruction()
            ->getOpcode() != llvm::Instruction::Br) {
      cc->builder->CreateBr(mergeB);
    }
    cc->blocks.pop_back();
  }

  f->getBasicBlockList().push_back(mergeB);
  cc->builder->SetInsertPoint(mergeB);
}

void IfStatement::compile(Context *cc){
  auto is = this; // TODO lazy
  // TODO context handling???
  is->exp->compile(cc);

  cc->blocks.push_back(getIB());
  is->i->compile(cc);
  cc->blocks.pop_back();

  if (is->e){
    cc->blocks.push_back(getEB());
    is->e->compile(cc);
    cc->blocks.pop_back();
  }
}


IfContext *IfStatement::getIB(llvm::BasicBlock *bb){
  if(!ib)
    ib = new IfContext();

  if(bb)
    ib->bblock = bb;

  return ib;
}

IfContext *IfStatement::getEB(llvm::BasicBlock *bb){
  if(!eb)
    eb = new IfContext();

  if(bb)
    eb->bblock = bb;

  return eb;
}
