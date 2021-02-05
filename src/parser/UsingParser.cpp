#include "UsingParser.h"
#include "Keywords.h"
#include "Symbols.h"
#include "../ast/Using.h"

UsingToken::UsingToken(Token *exp, CC *cc, Source *s, int spos, int epos)
  : exp(exp)
{
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

std::string UsingToken::desc(){
  return "using " + exp->desc();
}

AST *UsingToken::toAST(CC *cc){
  return new Using((Expression *) exp->getAST(cc));
}

ParseResult UsingParser::scheme(CC *cc, Source *s, int pos){
  auto ans = ukp.parse(cc, s, pos);
  auto exp = ans >> ep;
  ans = exp >> sp;

  if(!ans) return ans;
  return ParseResult(new UsingToken(((TupleToken *) exp.token)->t2, cc, s, ans.token->spos, ans.token->epos));
}

UsingParser::UsingParser()
  : ukp(k_using)
  , sp(s_semicolon)
{}
