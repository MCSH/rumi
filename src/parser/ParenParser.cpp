#include "ParenParser.h"
#include "Symbols.h"

ParenToken::ParenToken(Token *exp, CC *cc, Source *s, int spos, int epos)
  : exp(exp)
{
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

void *ParenToken::get(std::string key){
  if(key == "exp"){
    return exp;
  }
  // TODO Error?
  return 0;
}

std::string ParenToken::desc(){
  return "("+ exp->desc() + ")";
}

AST *ParenToken::toAST(CC *cc){
  return exp->toAST(cc);
}

ParenParser::ParenParser()
  : lpsp(s_lpar)
  , rpsp(s_rpar)
{}

ParseResult ParenParser::scheme(CC *cc, Source *s, int pos){
  auto tmp = lpsp.parse(cc, s, pos) >> ep;
  auto ans = tmp >> rpsp;
  if(!ans) return ans;
  return ParseResult(new ParenToken(((TupleToken*) tmp.token)->t2, cc, s, ans.token->spos, ans.token->epos));
}

int ParenParser::prec(){
  return 0;
}
