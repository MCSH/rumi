#include "Directive.h"
#include "../LLContext.h"
#include "../base.h"
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>

Directive::Directive(std::string id, AST *top)
  : id(id)
  , top(top)
{}

void Directive::prepare(CC *cc){
  if(id != "compile"){
    cc->debug(NONE) << "Directive  " << id << " Not implemented" << std::endl;
    exit(1);
  }

  // assuming @compile
  f = dynamic_cast<Function *>(top);
  if(!f){
    cc->debug(NONE) << "@compile only works on functions" << std::endl;
    exit(1);
  }

  f->prepare(cc);
}

void Directive::compile(CC *cc){
  f->compile(cc);
}

void Directive::codegen(CC *cc){
  f->codegen(cc);

  std::string e;
  llvm::EngineBuilder EB(std::unique_ptr<llvm::Module>(cc->llc->module));
  EB.setErrorStr(&e);
  llvm::ExecutionEngine *EE = EB.create();

  if(!EE){
    cc->debug(NONE) << "There was a problem initializing the EE" << std::endl << e << std::endl;
  }


  int (*faddress)() = (int (*) ()) EE->getFunctionAddress(f->id);

  int retval = faddress();

  EE->removeModule(cc->llc->module);
}
