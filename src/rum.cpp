#include "ast/Statement.h"
#include "base.h"
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/Verifier.h>
#include <ostream>

#include "parser/parser.h"
#include "Source.h"
#include "LLContext.h"

#include "ast/ast.h"

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

int main(int argc, char **argv) {
  CompileContext cc(argc, argv);

  for (Source *s : cc.sources) {
    cc.debug(Verbosity::MEDIUM) << *s << std::endl;
  }

  cc.debug(Verbosity::MEDIUM) << "Context created." << std::endl;

  cc.llc = new LLContext();

  for(Source *s: cc.sources){
    //auto s_ast =
    s->parse(&cc);
    //asts.insert(asts.end(), s_ast->begin(), s_ast->end());
    //delete s_ast;
  }

  /*
  for(auto a: asts)
      a->prepare(&cc);
  for(auto a: asts)
      a->compile(&cc);
  */
  for(int i = cc.ast_gened + 1; i < cc.asts.size(); i++){
    AST *a = cc.asts[i];
    if (Statement *s = dynamic_cast<Statement *>(a))
      s->codegen(&cc);
    cc.ast_gened = i;
  }

  if(cc.removeMeta){
    for (std::string f : cc.metaFunctions) {
      cc.debug(LOW) << "Removing function " << f << std::endl;
      auto ff = cc.llc->module->getFunction(f);
      if(ff) ff->removeFromParent();
    }
  }

  if(cc.verbosity >= Verbosity::LOW)
    cc.llc->module->print(llvm::outs(), nullptr);

  llvm::verifyModule(*cc.llc->module);

  auto targetTriple = llvm::sys::getDefaultTargetTriple();

  std::string Error;
  auto target = llvm::TargetRegistry::lookupTarget(targetTriple, Error);

  if (!target) {
    llvm::errs() << Error;
    exit(1);
  }

  auto CPU = "generic";
  auto Features = "";

  llvm::TargetOptions opt;
  auto RM = llvm::Optional<llvm::Reloc::Model>(
      llvm::Reloc::Model::PIC_); // Make it PIC
  auto targetMachine =
      target->createTargetMachine(targetTriple, CPU, Features, opt, RM);

  cc.llc->module->setDataLayout(targetMachine->createDataLayout());
  cc.llc->module->setTargetTriple(targetTriple);

  std::error_code EC;
  auto output = cc.outFile;
  llvm::raw_fd_ostream dest(output, EC, llvm::sys::fs::OF_None);

  if (EC) {
    llvm::errs() << "Could not open file: " << EC.message();
    exit(1);
  }

  llvm::legacy::PassManager pass;
  // auto fileType = llvm::TargetMachine::CGFT_ObjectFile;
  auto fileType = llvm::CGFT_ObjectFile;

  if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
    llvm::errs() << "Target Machine can't emit a file of this type";
    exit(1);
  }

  pass.run(*(cc.llc->module));
  dest.flush();

  // Call linker to create the executable
  if(cc.callLinker){
    std::string command = "clang -o " + cc.execFile + " " + cc.outFile;

    for(auto s: cc.linkerArgs){
      command += " " + s;
    }
    std::system(command.c_str());
  }

  return 0;
}
