#include "LLContext.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/Support/TargetSelect.h"

LLContext::LLContext(){
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  this->module = new llvm::Module("my cool jit", this->context);
  this->builder = new llvm::IRBuilder<>(this->context);
}
