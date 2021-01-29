#include "DynamicParseRule.h"
#include "../base.h"
#include "../LLContext.h"
#include "Token.h"
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>

DynamicParseRule::DynamicParseRule(CC *cc, std::string name)
  : name(name)
{
  // figure out p
  p = 0;
  
  // figure out innerf
  auto fname = name + "$parse";

  std::string e;
  llvm::EngineBuilder EB(std::unique_ptr<llvm::Module>(cc->llc->module));
  EB.setErrorStr(&e);
  llvm::ExecutionEngine *EE = EB.create();

  innerf = (ParseResult *(*)(void *, CC *, Source *, int)) EE->getFunctionAddress(fname);

  if(!innerf){
    cc->debug(NONE) << "Parser " << name << " was not declared properly, missing method parse" << std::endl;
    exit(1);
  }
}

ParseResult DynamicParseRule::scheme(CC *cc, Source *s, int pos){
  auto ans = innerf(p, cc, s, pos);

  if(!ans) return 0;

  return ParseResult(new DynamicParseToken(this, ans->token));
}

DynamicParseToken::DynamicParseToken(DynamicParseRule *drp, Token *res)
  : drp(drp)
  , res(res)
{
  this->cc = res->cc;
  this->s = res->s;
  this->spos = res->spos;
  this->epos = res->epos;
}

std::string DynamicParseToken::desc(){
  return "Token of " + drp->name;
}

AST* DynamicParseToken::toAST(CC *cc){
  auto fname = drp->name + "$genAST";

  std::string e;
  llvm::EngineBuilder EB(std::unique_ptr<llvm::Module>(cc->llc->module));
  EB.setErrorStr(&e);
  llvm::ExecutionEngine *EE = EB.create();

  auto genAST = (AST *(*)(void *, CC *, ParseResult *)) EE->getFunctionAddress(fname);

  if(!genAST){
    cc->debug(NONE) << "Parser " << drp->name << " was not declared properly, missing method genAST" << std::endl;
    exit(1);
  }
  auto ast = genAST(drp->p, cc, new ParseResult(res));
  return (AST *)ast;
}
