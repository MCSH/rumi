#include "AssignParser.h"
#include "Symbols.h"
#include "../ast/Assign.h"

AssignToken::AssignToken(std::string id, Token *value, CC *cc, Source *s, int pos,
                                   int epos)
  : id(id)
  , value(value)
{
  this->cc = cc;
  this->s = s;
  this->spos = pos;
  this->epos = epos;
}

AST *AssignToken::toAST(CC* cc){
  return new Assign(id, (Expression *)value->toAST(cc));
}

std::string AssignToken::desc(){
  return id + " = " + value->desc();
}

ParseResult AssignParser::scheme(CC *cc, Source *s, int pos){
  auto tid = ip.parse(cc, s, pos);
  auto ans = tid >> esp >> vp;
  if(!ans) return ans;
  return ParseResult(new AssignToken(
                                     ((IdToken*)tid.token)->id,
                                     ((TupleToken*) ans.token)->t2,
                                     cc, s,
                                     ans.token->spos,
                                     ans.token->epos));
}

AssignParser::AssignParser()
  : esp(s_eq)
{}
