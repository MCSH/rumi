#include "AddressParser.h"
#include "../Source.h"
#include "Symbols.h"
#include "../ast/Address.h"

AddressToken::AddressToken(Token *exp, CC *cc, Source *s, int spos, int epos)
  : exp(exp)
{
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

void *AddressToken::get(std::string key){
  if(key == "exp"){
    return exp;
  }
  // TODO Error?
  return 0;
}

AST *AddressToken::toAST(CC *cc){
  return new Address((Expression *) exp->toAST(cc));
}

std::string AddressToken::desc(){
  return "&" + exp->desc();
}

ParseResult AddressParser::scheme(CC *cc, Source *s, int pos){
  ParseState *ps = s->resolveState(pos);
  if(ps->hasParser("address")) return ps->getToken("address"); // recursive
  ps->setToken("address", 0);

  Token* tmp =  innerscheme(cc, s, pos).token;

  ps->setToken("address", tmp);
  return ParseResult(tmp);
}

ParseResult AddressParser::innerscheme(CC *cc, Source *s, int pos){
  auto ans = sp.parse(cc, s, pos) >> ep;
  if(!ans) return ans;
  return ParseResult(new AddressToken(((TupleToken *) ans.token)->t2, cc, s, ans.token->spos, ans.token->epos));
}

AddressParser::AddressParser()
  : sp(s_and)
{}
