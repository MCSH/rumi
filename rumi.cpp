#include "compiler.h"
#include "codegen.h"
#include <llvm/IR/DataLayout.h>
#include <stdio.h>
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include <unistd.h>

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

  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();
  LLVMInitializeNativeAsmParser();


  char *cwd = get_current_dir_name();
  auto statements = compile(argv[1]);
  chdir(cwd); // Compile will change cwd, so go back for mod generating.
  auto cc = codegen(statements, getModuleName(argv[1]), false, false);

  llvm::ExecutionEngine *EE = llvm::EngineBuilder(std::move(cc->module)).create();

  auto main = cc->mainF;


  std::vector<llvm::GenericValue> noargs;
  llvm::GenericValue gv = EE->runFunction(main, noargs);

  return gv.IntVal.getLimitedValue();
}
