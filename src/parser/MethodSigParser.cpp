#include "MethodSigParser.h"
#include "Symbols.h"
#include "../ast/Method.h"

MethodSigToken::MethodSigToken(std::string structName, std::string methodName, Token *f, CC *cc, Source *s, int spos, int epos)
  : structName(structName)
  , methodName(methodName)
  , f(f)
{
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

AST* MethodSigToken::toAST(CC *cc){
  auto ff = (FunctionSig *) f->getAST(cc);
  auto mf = new Method(structName, methodName, new Function()); // This is fine, we will have replaced it by the time we call, hopefully!
  mf->f->id = ff->id;
  mf->f->returnType = ff->returnType;
  mf->f->args = ff->args;
  mf->f->isDummy = true;
  return mf;
}

std::string MethodSigToken::desc(){
  return structName + "." + methodName + ": " + f->desc();
}

ParseResult MethodSigParser::scheme(CC *cc, Source *s, int pos){
  auto structAns = idp.parse(cc, s, pos);
  auto methodAns = structAns >> dsp >> idp;
  auto ans = methodAns >> csp >> esp >> fsp;
  if(!ans) return ans;

  std::string sid = ((IdToken *) structAns.token)->id;
  std::string mid = ((IdToken *) ((TupleToken* ) methodAns.token)->t2)->id;
  Token *f = ((TupleToken*) ans.token) -> t2;

  return ParseResult(new MethodSigToken(sid, mid, f, cc, s, ans.token->spos, ans.token->epos));
}

MethodSigParser::MethodSigParser()
  : dsp(s_dot)
  , csp(s_col)
  , esp(s_eq)
{}
