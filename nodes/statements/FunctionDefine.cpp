#include "FunctionDefine.h"
#include "../../Context.h"
#include "../types/VoidType.h"

FunctionDefine::~FunctionDefine() {
  delete sign;
  delete body;
}
void FunctionDefine::codegen(CodegenContext *cc) { funcgen(cc); }

llvm::Function *FunctionDefine::funcgen(CodegenContext *cc) {
  // TODO

  cc->defered.push_back(new std::vector<Statement *>());

  auto f = this->sign->signgen(cc);
  auto fargs = this->sign->args;

  unsigned idx = 0;
  for (auto &arg : f->args()) {
    if ((*fargs)[idx]->vardiac)
      break;
    arg.setName(*(*fargs)[idx++]->name);
  }

  auto preIp = cc->builder->GetInsertBlock();

  llvm::BasicBlock *bblock = llvm::BasicBlock::Create(cc->context, "entry", f);
  cc->builder->SetInsertPoint(bblock);
  cc->block.push_back(new CodegenBlockContext(bblock));

  llvm::BasicBlock *endblock = llvm::BasicBlock::Create(cc->context, "end", f);
  auto cbc = cc->block.back();
  cbc->endblock = endblock;

  bool isVoid =
      typeid(*this->sign->returnT).hash_code() == typeid(VoidType).hash_code();

  llvm::AllocaInst *ra;

  // TODO check for void
  if (!isVoid) {
    ra = cc->builder->CreateAlloca(this->sign->returnT->typeGen(cc), 0,
                                   "returnval");
    cbc->returnAlloca = ra;
  }

  // stackrestore at end, before returns
  llvm::Value *ss = nullptr;
  if (this->dynamicStack) {
    ss = cc->builder->CreateCall(llvm::Intrinsic::getDeclaration(
        cc->module, llvm::Intrinsic::stacksave));
    std::vector<llvm::Value *> a;
    a.push_back(ss);
    cc->builder->SetInsertPoint(endblock);
    cc->builder->CreateCall(llvm::Intrinsic::getDeclaration(
                                cc->module, llvm::Intrinsic::stackrestore),
                            a);
  }
  cc->builder->SetInsertPoint(bblock);

  // handle arguments
  int i = 0;
  for (auto &arg : f->args()) {
    // TODO is it redundent to convert arg to alloca and store?
    // TODO varargs
    auto a = (*fargs)[i]; // This argument
    llvm::AllocaInst *alloc =
        cc->builder->CreateAlloca(a->t->typeGen(cc), 0, *a->name);
    cc->setVariable(a->name, alloc, a);
    cc->builder->CreateStore(&arg, alloc);
    i++;
  }

  // generate body
  for (auto s : *this->body->stmts) {
    s->codegen(cc);
  }

  // TODO we should raise an error for return-less functions
  // cc->builder->CreateBr(endblock);
  if (cc->builder->GetInsertPoint()
          ->getPrevNonDebugInstruction()
          ->getOpcode() != llvm::Instruction::Br) {
    cc->builder->CreateBr(endblock);
  }

  cc->builder->SetInsertPoint(endblock);

  handleDefer(cc);

  if (isVoid)
    cc->builder->CreateRetVoid();
  else
    cc->builder->CreateRet(cc->builder->CreateLoad(ra));

  llvm::verifyFunction(*f);

  cc->block.pop_back();

  if (this->sign->name->compare("main") == 0) {
    cc->mainF = f;
  }

  cc->builder->SetInsertPoint(preIp);

  return f;
}

void FunctionDefine::compile(CompileContext *cc) {
  FunctionDefine *fd = this; // TODO because I'm lazy
  cc->blocks.push_back(new BlockContext());

  fd->sign->noBlockCompile(cc);
  // functionSignCompile(fd->sign, cc); // TODO is this fine?, WRONG!

  cc->getBlock()->currentFunction = fd;

  for (auto s : *fd->body->stmts) {
    s->compile(cc);
  }

  cc->blocks.pop_back();
}
