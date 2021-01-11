#include "PtrAssignParser.h"
#include "Symbols.h"
#include "../ast/PtrAssign.h"

PtrAssignToken::PtrAssignToken(Token *ptr, Token *value, CC *cc, Source *s, int spos, int epos)
  : ptr(ptr)
  , value(value)
{
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

std::string PtrAssignToken::desc(){
  return "*" + ptr->desc() + " = " + value->desc();
}

AST *PtrAssignToken::toAST(CC *cc){
  return new PtrAssign((Expression *) ptr->toAST(cc), (Expression*) value->toAST(cc));
}

PtrAssignParser::PtrAssignParser()
  : esp(s_eq)
  , ssp(s_mult)
  , ep(5)
{}

ParseResult PtrAssignParser::scheme(CC *cc, Source *s, int pos){
  auto tmp = ssp.parse(cc, s, pos) >> ep;
  auto ans = tmp >> esp >> vp;
  if(!ans) return ans;
  Token *ptr = ((TupleToken*) tmp.token) -> t2;
  Token *value = ((TupleToken*)ans.token) -> t2;
  return ParseResult(new PtrAssignToken(ptr, value, cc, s, ans.token->spos, ans.token->epos));
}
