#include "LLContext.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/Support/TargetSelect.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instruction.h>

LLContext::LLContext(){
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  this->module = new llvm::Module("my cool jit", this->context);
  this->builder = new llvm::IRBuilder<>(this->context);
}

bool LLContext::isBrOk(llvm::IRBuilder<> *builder){
  if(!builder) builder = this->builder;

  if(builder->GetInsertBlock()->empty()) return true;

  auto lastop = builder->GetInsertPoint()->getPrevNonDebugInstruction()->getOpcode();

  if(lastop == llvm::Instruction::Br) return false;
  if(lastop == llvm::Instruction::Ret) return false;

  return true;
}
