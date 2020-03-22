#include "MemberExpr.h"
#include "../../Context.h"
#include "../types/PointerType.h"
#include "../types/StructType.h"

MemberExpr::~MemberExpr() {
  delete e;
  delete mem;
}
llvm::Value *MemberExpr::exprGen(CodegenContext *cc) {
  auto member_ptr = this->getAlloca(cc); // Level is handled there
  llvm::Value *loaded_member = cc->builder->CreateLoad(member_ptr, "loadtmp");
  return loaded_member;
}

llvm::Value *MemberExpr::getAlloca(CodegenContext *cc) {
  int member_ind = 0;

  // TODO level

  // maybe it's not StructType
  auto tmpe = this->e->exprType;

  while (PointerType *p = dynamic_cast<PointerType *>(tmpe)) {
    tmpe = p->base;
  }

  auto t = (StructType *)tmpe;
  auto ss = cc->getStructStruct(t->name);
  auto st = (llvm::StructType *)cc->getStructType(t->name);

  for (auto m : *ss->members) {
    if (m->name->compare(*this->mem) == 0) {
      break;
    }
    member_ind++;
  }

  member_ind++; // first one is vptr

  llvm::Value *member_index =
      llvm::ConstantInt::get(cc->context, llvm::APInt(32, member_ind, true));

  llvm::Value *alloc;

  alloc = this->e->getAlloca(cc);

  for (int i = 0; i < this->level; i++) {
    alloc = cc->builder->CreateLoad(alloc, "paccess");
  }

  std::vector<llvm::Value *> indices(2);
  indices[0] = llvm::ConstantInt::get(cc->context, llvm::APInt(32, 0, true));
  indices[1] = member_index;

  llvm::Value *member_ptr =
      cc->builder->CreateInBoundsGEP(st, alloc, indices, "memberptr");
  return member_ptr;
}

void MemberExpr::compile(Context *cc) {
  resolveType(cc);
  e->compile(cc);
  // TODO check to see if mem is in the struct
  auto tmpe = e->resolveType(cc);

  int level = 0;

  while (PointerType *p = dynamic_cast<PointerType *>(tmpe)) {
    tmpe = p->base;
    level++;
  }
  this->level = level;
}

Type *MemberExpr::resolveType(Context *cc) {
  if (exprType)
    return exprType;
  // TODO check to see if it's a pointer
  auto tmpe = this->e->resolveType(cc);

  while (PointerType *p = dynamic_cast<PointerType *>(tmpe)) {
    tmpe = p->base;
  }

  auto t = (StructType *)tmpe;
  auto ss = cc->getStruct(t->name);

  // error when ss not found
  if (!ss) {
    printf("Struct %s was not found, line %d\n", t->name->c_str(),
           this->lineno);
    exit(1);
  }

  for (auto m : *ss->members) {
    if (m->name->compare(*this->mem) == 0) {
      this->exprType = m->t->clone();
      return this->exprType;
    }
  }
  printf("Member type not found, memberExprType on line %d\n", lineno);
  exit(1);
  return nullptr;
}
