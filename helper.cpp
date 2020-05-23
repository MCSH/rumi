#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/TargetRegistry.h"
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

extern "C" 
void CompileLLVM(llvm::Module *module, char *output_c) {

  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  auto targetTriple = llvm::sys::getDefaultTargetTriple();

  std::string Error;
  auto target = llvm::TargetRegistry::lookupTarget(targetTriple, Error);

  if (!target) {
    llvm::errs() << Error;
    return;
  }

  auto CPU = "generic";
  auto Features = "";

  llvm::TargetOptions opt;
  auto RM = llvm::Optional<llvm::Reloc::Model>(
      llvm::Reloc::Model::PIC_); // Make it PIC
  auto targetMachine =
      target->createTargetMachine(targetTriple, CPU, Features, opt, RM);

  module->setDataLayout(targetMachine->createDataLayout());
  module->setTargetTriple(targetTriple);

  std::error_code EC;
  auto output = std::string(output_c);
  llvm::raw_fd_ostream dest(output, EC, llvm::sys::fs::F_None);

  if (EC) {
    llvm::errs() << "Could not open file: " << EC.message();
    return;
  }

  llvm::legacy::PassManager pass;
  // auto fileType = llvm::TargetMachine::CGFT_ObjectFile;
  auto fileType = llvm::CGFT_ObjectFile;

  if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
    llvm::errs() << "Target Machine can't emit a file of this type";
    return;
  }

  pass.run(*(module));
  dest.flush();
}
