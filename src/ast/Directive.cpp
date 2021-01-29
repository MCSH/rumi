#include "Directive.h"
#include "../LLContext.h"
#include "../base.h"
#include "ast.h"
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include "../DebugInfo.h"

Directive::Directive(std::string id, AST *top)
  : id(id)
  , top(top)
{}

void Directive::prepare(CC *cc){
  if(id != "compile"){
    graceFulExit(dbg, "Directive " + id + " not implemented");
  }

  // assuming @compile
  f = dynamic_cast<Function *>(top);
  if(!f){
    graceFulExit(dbg, "@compile works only on functions");
  }

  f->prepare(cc);
}

void Directive::compile(CC *cc){
  f->compile(cc);
  cc->metaFunctions.insert(f->id);

  cc->debug(LOW) << "Calling codegen from directive on " << cc->asts.size() - cc->ast_gened << std::endl;
  for(int i = cc->ast_gened + 1; i < cc->asts.size(); i++){
    AST * a = cc->asts[i];
    if(Statement *s = dynamic_cast<Statement *>(a)){
      s->codegen(cc);
    }
    cc->ast_gened = i;
  }
  
  f->codegen(cc);

  std::string e;
  llvm::EngineBuilder EB(std::unique_ptr<llvm::Module>(cc->llc->module));
  EB.setErrorStr(&e);
  llvm::ExecutionEngine *EE = EB.create();

  if(!EE){
    cc->debug(NONE) << "There was a problem initializing the EE" << std::endl << e << std::endl;
  }

  int retval;
  bool isRan = false;

  if(f->args.size() == 0){
    int (*faddress)() = (int (*) ()) EE->getFunctionAddress(f->id);
    retval = faddress();
    isRan = true;
  } else if(f->args.size() == 1){
    // pass in "compiler"
    void *c = cc->getCompileObj(EE);
    int (*faddress)(void*) = (int (*) (void*)) EE->getFunctionAddress(f->id);
    retval = faddress(c);
    isRan = true;
  }

  if(!isRan){
    graceFulExit(dbg, "Compile directive " +  f->id + " doesn't have a correct signature");
  }

  if(retval == 1){
    // WARNING
    cc->debug(NONE) << "Compile directive "<< f->id << " returned warning" << std::endl;
  } else if(retval > 1){
    // ERROR
    cc->debug(NONE) << "Compile directive " << f->id << " returned " << retval << " " << *dbg << std::endl;
    exit(retval);
  }

  EE->removeModule(cc->llc->module);
}

void Directive::codegen(CC *cc){
}
