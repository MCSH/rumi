#include "compiler.h"
#include "codegen.h"
// #include <llvm/ExecutionEngine/Orc/CompileUtils.h>
// #include <llvm/ExecutionEngine/Orc/Core.h>
// #include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
// #include <llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h>
// #include <llvm/ExecutionEngine/Orc/ObjectTransformLayer.h>
// #include <llvm/ExecutionEngine/RTDyldMemoryManager.h>
#include <llvm/IR/DataLayout.h>
#include <stdio.h>
// #include "llvm/ExecutionEngine/Orc/LLJIT.h"
// #include "llvm/Support/Error.h"
// #include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
// #include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"

std::string getModuleName(char *name){
  std::string ans(name);
  return ans.append(".o");
}

int main(int argc, char **argv){
  // TODO lookup yypush_parse
  if(argc != 2){
    printf("No file, abort\n");
    return 1;
  }
  auto statements = compile(argv[1]);
  auto cc = codegen(statements, getModuleName(argv[1]), false);

  llvm::ExecutionEngine *EE = llvm::EngineBuilder(std::move(cc->module)).create();

  auto main = cc->mainF;


  std::vector<llvm::GenericValue> noargs;
  llvm::GenericValue gv = EE->runFunction(main, noargs);

  return gv.IntVal.getLimitedValue();
}
