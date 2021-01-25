#include "MethodParser.h"
#include "Symbols.h"
#include "../ast/Method.h"

MethodToken::MethodToken(std::string structName, std::string methodName, Token *f, CC *cc, Source *s, int spos, int epos)
  : structName(structName)
  , methodName(methodName)
  , f(f)
{
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

AST* MethodToken::toAST(CC *cc){
  return new Method(structName, methodName, (Function *)f->toAST(cc));
}

std::string MethodToken::desc(){
  return structName + "." + methodName + ": " + f->desc();
}

ParseResult MethodParser::scheme(CC *cc, Source *s, int pos){
  auto structAns = idp.parse(cc, s, pos);
  auto methodAns = structAns >> dsp >> idp;
  auto ans = methodAns >> csp >> esp >> fp;
  if(!ans) return ans;

  std::string sid = ((IdToken *) structAns.token)->id;
  std::string mid = ((IdToken *) ((TupleToken* ) methodAns.token)->t2)->id;
  Token *f = ((TupleToken*) ans.token) -> t2;

  return ParseResult(new MethodToken(sid, mid, f, cc, s, ans.token->spos, ans.token->epos));
}

MethodParser::MethodParser()
  : dsp(s_dot)
  , csp(s_col)
  , esp(s_eq)
{}
