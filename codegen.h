#pragma once

#include "Context.h"

Context *codegen(std::vector<Statement *> *statements,
                        std::string outfile, bool print, bool ofile=true);

llvm::Value *castGen(Type *exprType, Type *baseType, llvm::Value *e, Context *cc, Node *n, bool expl, llvm::AllocaInst *alloc=nullptr);

void handleDefer(Context *cc);

llvm::Value* arrayToPointer(llvm::Type* t, llvm::Value * alloc, Context *cc);
