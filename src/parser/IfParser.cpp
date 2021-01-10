#include "IfParser.h"
#include "Symbols.h"
#include "Keywords.h"
#include "../ast/If.h"

IfToken::IfToken(Token *value, Token *st1, Token *st2, CC *cc, Source *s, int spos, int epos)
  : value(value)
  , st1(st1)
  , st2(st2)
{
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

AST *IfToken::toAST(CC *cc){
  return new If((Expression*)value->toAST(cc), (Statement*)st1->toAST(cc), (Statement*)st2->toAST(cc));
}

std::string IfToken::desc(){
  return "if " + value->desc() + "\n        " + st1->desc()
    + (st2? "\n        " +  st2->desc() : "");
}

ParseResult IfParser::scheme(CC *cc, Source *s, int pos){
  auto vt = ip.parse(cc, s, pos) >> lpar >> vp;
  auto i = vt >> rpar >> sp;

  if(!i) return i;

  Token *value = ((TupleToken*) vt.token) -> t2;
  Token *st1 = ((TupleToken*) i.token)->t2;
  Token *st2 = 0;

  auto tmp = i >> ep >> sp;
  if(tmp){
    st2 = ((TupleToken*) tmp.token) -> t2;
    i = tmp;
  }
  return ParseResult(new IfToken(value, st1, st2, cc, s, i.token->spos, i.token->epos));
}


IfParser::IfParser()
  : lpar(s_lpar)
  , rpar(s_rpar)
  , ip(k_if)
  , ep(k_else)
{}
