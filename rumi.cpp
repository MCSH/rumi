#include "compiler.h"
#include "codegen.h"
#include <llvm/IR/DataLayout.h>
#include <stdio.h>
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include <unistd.h>

#include "cli.h"

std::string getModuleName(char *name){
  std::string ans(name);
  return ans.append(".o");
}

int main(int argc, char **argv){

  struct arguments arguments;

  arguments.output_file = 0;
  arguments.llvm_ir = false;

  argp_parse (&argp, argc, argv, 0, 0, &arguments);

  if(!arguments.output_file){
    arguments.output_file = new std::string(getModuleName(arguments.input_file));
  }

  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();
  LLVMInitializeNativeAsmParser();


  char *cwd = get_current_dir_name();
  auto co = compile(arguments.input_file);
  chdir(cwd); // Compile will change cwd, so go back for mod generating.
  auto cc = codegen(co->codes, *arguments.output_file, arguments.llvm_ir, false);


  llvm::ExecutionEngine *EE = llvm::EngineBuilder(std::unique_ptr<llvm::Module>(cc->module)).create();

  auto main = cc->mainF;


  std::vector<llvm::GenericValue> noargs;
  llvm::GenericValue gv = EE->runFunction(main, noargs);

  return gv.IntVal.getLimitedValue();
}
