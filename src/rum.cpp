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
    cc.debug(Verbosity::LOW) << *s << std::endl;
  }

  cc.debug(Verbosity::MEDIUM) << "Context created." << std::endl;

  cc.llc = new LLContext();

  std::vector<AST *> asts;

  for(Source *s: cc.sources){
    cc.load(s);
    ParseResult t;
    t = cc.parser.parseTop(s);
    while(t){
      AST *a = t.token->toAST(&cc);
      if(a) asts.push_back(a);
      cc.debug(Verbosity::NONE) << t << std::endl;
      t = cc.parser.parseTop(s, t.token->epos + 1);
    }
  }

  for(AST *a: asts){
    a->prepeare(&cc);
  }
  for(AST *a: asts){
    a->compile(&cc);
  }
  for(AST *a: asts){
    if(Statement *s = dynamic_cast<Statement *>(a))
      s->codegen(&cc);
  }

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
  auto output = "out.o";
  llvm::raw_fd_ostream dest(output, EC, llvm::sys::fs::F_None);

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

  return 0;
}
