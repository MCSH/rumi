#include "Function.h"
#include "Statement.h"
#include <llvm/IR/BasicBlock.h>
#include "../base.h"
#include "../LLContext.h"
#include "Type.h"

void Function::prepeare(CC *cc){
  for(Statement *s: statements){
    for(AST *dep: s->compileDeps){
      compileDeps.insert(dep);
    }
  }

  // TODO args, returnType

  for(Statement *s: statements){
    s->prepeare(cc);
  }
}

void Function::compile(CC *cc){
  for(Statement *s: statements){
    for(AST *dep: s->linkDeps){
      linkDeps.insert(dep);
    }
  }
  // TODO args, returnType

  for(Statement *s: statements){
    s->compile(cc);
  }
}

void Function::codegen(CC *cc){
  // TODO
  llvm::Type* rt = returnType?(llvm::Type*)returnType->typegen(cc):
    llvm::Type::getVoidTy(cc->llc->context);

  auto ft = llvm::FunctionType::get(rt, false);

  auto f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, this->id, cc->llc->module);

  auto entry = llvm::BasicBlock::Create(cc->llc->context, "entry", f);

  cc->llc->builder->SetInsertPoint(entry);

  for(Statement *s: statements){
    s->codegen(cc);
  }
}

void Function::setId(std::string id){
  this->id = id;
}
