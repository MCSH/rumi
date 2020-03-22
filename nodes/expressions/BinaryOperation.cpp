#include "BinaryOperation.h"
#include "../../Context.h"
#include "../types/FloatType.h"
#include "../types/IntType.h"
#include "../types/PointerType.h"
#include <llvm/IR/Instruction.h>

BinaryOperation::~BinaryOperation() {
  delete lhs;
  delete rhs;
}
llvm::Value *BinaryOperation::exprGen(CodegenContext *cc){
  auto *lhs = this->lhs->exprGen(cc);
  auto *rhs = this->rhs->exprGen(cc);

  auto lt = typeid(*this->lhs->exprType).hash_code();
  auto rt = typeid(*this->rhs->exprType).hash_code();

  auto it = typeid(IntType).hash_code();
  auto ft = typeid(FloatType).hash_code();

  auto pt = typeid(PointerType).hash_code();

  if (op == EQ){
    return cc->builder->CreateICmpEQ(lhs, rhs);
  }

  // TODO change this based on type somehow?
  if((lt == it || lt == ft) && (rt== it || rt == ft)){
    llvm::Instruction::BinaryOps instr;

    switch (this->op) {
    case Operation::PLUS:
      instr = llvm::Instruction::Add;
      break;
    case Operation::SUB:
      instr = llvm::Instruction::Sub;
      break;
    case Operation::MULT:
      instr = llvm::Instruction::Mul;
      break;
    case Operation::DIV:
      instr = llvm::Instruction::SDiv;
      break;
    case Operation::MOD:
      instr = llvm::Instruction::SRem;
      break;
    }

    return cc->builder->CreateBinOp(instr, lhs, rhs);
  }

  if(((pt == lt && rt == it) || (pt == it && rt == pt)) &&
     (this->op == PLUS || this->op == SUB)){
    // TODO handle the type in compiler?
    // TODO handling pointer

    // Set lhs to the pointer
    if(rt == pt){
      auto tmp = lhs;
      lhs = rhs;
      rhs = tmp;

      auto tmp2 = this->lhs;
      this->lhs = this->rhs;
      this->rhs = tmp2;
    }

    // TODO cast rhs to i64 (or 32 dep on arch)

    if(this->op == SUB){
      auto mone = llvm::ConstantInt::get(cc->context, llvm::APInt(64, -1, false));
      rhs = cc->builder->CreateBinOp(llvm::Instruction::Mul, rhs, mone);
    }

    auto t = this->lhs->exprType->typeGen(cc);

    std::vector<llvm::Value *> indices(1);
    indices[0] = rhs;
    return cc->builder->CreateInBoundsGEP(t->getPointerElementType(), lhs, indices, "ptrarrptr");
  }

  printf("Can't run the expression on types %s and %s on line %d\n", this->lhs->exprType->displayName().c_str(), this->rhs->exprType->displayName().c_str(), this->lineno);
}

llvm::Value *BinaryOperation::getAlloca(CodegenContext *cc) {}

void BinaryOperation::compile(Context *cc){
  resolveType(cc);
  lhs->compile(cc);
  rhs->compile(cc);
  if(op == EQ){
    // TODO ensure type mathces
    if(lhs->exprType->compatible(rhs->exprType) > ExpCast){
      printf("Can not compare types %s and %s, line %d\n", lhs->exprType->displayName().c_str(), rhs->exprType->displayName().c_str(), lineno);
      exit(1);
    }
  }
  // TODO check to see if the op is valid on this type.
  // TODO
  // TODO I'd rather handle the op identification here, but it needs discussion
}

Type *BinaryOperation::resolveType(Context *cc){
  if(exprType)
    return exprType;

  if(op == EQ){
    exprType = new IntType(1);
    return exprType;
  }

  // TODO improve!
  exprType =lhs->resolveType(cc)->clone();
  return exprType;
}
