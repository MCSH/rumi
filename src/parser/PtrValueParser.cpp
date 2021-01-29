#include "PtrValueParser.h"
#include "Symbols.h"
#include "../ast/PtrValue.h"

PtrValueToken::PtrValueToken(Token *exp, CC *cc, Source *s, int spos, int epos)
  : exp(exp)
{
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

std::string PtrValueToken::desc(){
  return "*" + exp->desc();
}

AST* PtrValueToken::toAST(CC *cc){
  return new PtrValue((Expression *) exp->getAST(cc));
}

PtrValueParser::PtrValueParser()
  : ssp(s_mult)
  , ep(5)
{}

ParseResult PtrValueParser::scheme(CC *cc, Source *s, int pos){
  auto ans = ssp.parse(cc, s, pos) >> ep;
  if(!ans) return ans;
  return ParseResult(new PtrValueToken(((TupleToken*) ans.token)->t2, cc, s, ans.token->spos, ans.token->epos));
}
