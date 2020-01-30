#include "VariableDecl.h"
#include "../../Context.h"
#include "../expressions/IntValue.h"
#include "../types/ArrayType.h"
#include "../types/InterfaceType.h"
#include "../types/StructType.h"
#include <llvm/IR/IRBuilder.h>

VariableDecl::~VariableDecl() {
  delete name;
  delete t;
  delete exp;
}

void VariableDecl::codegen(CodegenContext *cc) {
  // TODO maybe we need the block first for allocation?
  auto t = this->t->typeGen(cc);
  if (!t) {
    printf("Unknown type %s\n", ((StructType *)this->t)->name->c_str());
    exit(1);
  }

  auto bblock = cc->block.back()->bblock;

  if (auto at = dynamic_cast<ArrayType *>(this->t)) {
    if (at->exp) {
      // stacksave/stackrestore? TODO
      auto count = at->exp->exprGen(cc);
      count = castGen(at->exp->exprType, at->base, count, cc, this, true);
      auto alloc = cc->builder->CreateAlloca(t, 0, count, this->name->c_str());
      cc->setVariable(this->name, alloc, this);
      return;
    }
  }
  // Normal variable OR array of fixed size

  // TODO handle global
  llvm::IRBuilder<> TmpB(bblock, bblock->begin());

  llvm::AllocaInst *alloc = TmpB.CreateAlloca(t, 0, this->name->c_str());
  cc->setVariable(this->name, alloc, this);

  // TODO if not array, handle array with exp seperately
  if (this->exp) {
    // set variable to expr
    auto exp = this->exp->exprGen(cc);
    cc->builder->CreateStore(exp, alloc);
  }

  if (typeid(*this->t).hash_code() == typeid(StructType).hash_code()) {
    StructStatement *ss = cc->getStructStruct(((StructType *)this->t)->name);
    if (ss->has_initializer) {
      for (int i = 0; i < ss->members->size(); i++) {
        auto vd = (*ss->members)[i];
        if (vd->exp) {

          // We add +1 because 0 is vptr
          llvm::Value *member_index =
              llvm::ConstantInt::get(cc->context, llvm::APInt(32, i + 1, true));

          std::vector<llvm::Value *> indices(2);
          indices[0] =
              llvm::ConstantInt::get(cc->context, llvm::APInt(32, 0, true));
          indices[1] = member_index;

          llvm::Value *member_ptr =
              cc->builder->CreateInBoundsGEP(t, alloc, indices, "memberptr");

          cc->builder->CreateStore(vd->exp->exprGen(cc), member_ptr);
        }
      }
    }
  }
}

void VariableDecl::compile(CompileContext *cc) {
  auto vd = this; // TODO lazy
  Type *type;
  // check for arrays
  if (vd->t) {
    if (ArrayType *at = dynamic_cast<ArrayType *>(vd->t)) {
      if (at->exp || at->count) {
        if (auto *mt = dynamic_cast<IntValue *>(at->exp)) {
          at->count = atoi(mt->val->c_str());
          delete at->exp; // TODO problematic, remove in case of segfault
          at->exp = nullptr;
        } else {
          cc->getCurrentFunction()->dynamicStack = true;
          at->exp->compile(cc);
        }
      }
    }
  }
  // check exp type
  if (vd->exp) {
    if (vd->t) {
      vd->exp->compile(cc);
      auto etype = vd->exp->resolveType(cc);
      auto c = vd->t->compatible(etype);
      if (c == OK || c == ImpCast) {
        delete etype; // TODO problematic, remove in case of segfault
      } else {
        printf("Uncompatible type covnersion from %s to %s at line %d\n",
               etype->displayName().c_str(), vd->t->displayName().c_str(),
               vd->lineno);
        exit(1);
      }
      type = vd->t;
    } else {
      vd->exp->compile(cc);
      type = vd->exp->resolveType(cc);
    }
  } else {
    type = vd->t;
  }

  vd->t = type; // Just to ensure it is there, for use in codegen.

  // pointer level, how many pointers there are
  int level = 0;
  Type *tt = vd->t;
  while (PointerType *pt = dynamic_cast<PointerType *>(tt)) {
    tt = pt->base;
    level++;
  }

  // handle structs and interfaces
  if (typeid(*tt).hash_code() == typeid(StructType).hash_code()) {
    StructType *st = (StructType *)tt;
    if (StructStatement *ss = cc->getStruct(st->name)) {
      // nothing to worry about, keep going.
    } else if (InterfaceStatement *is = cc->getInterface(st->name)) {
      // replace with interface
      // TODO memory leak
      tt = new InterfaceType(is->name);
      while (level--)
        tt = new PointerType(tt);

      vd->t = tt;
    } else {
      // Struct or Interface not found
      printf("Unknown type %s on line %d\n", st->name->c_str(), vd->lineno);
      exit(1);
    }
  }

  // Add to block
  cc->getBlock()->newVar(vd->name, vd->t);
}
