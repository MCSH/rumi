#include "AssignParser.h"
#include "Symbols.h"
#include "../ast/Assign.h"

AssignToken::AssignToken(Token *expr, Token *value, CC *cc, Source *s, int pos,
                                   int epos)
  : expr(expr)
  , value(value)
{
  this->cc = cc;
  this->s = s;
  this->spos = pos;
  this->epos = epos;
}

AST *AssignToken::toAST(CC* cc){
  return new Assign((Expression *) expr->getAST(cc), (Expression *)value->getAST(cc));
}

std::string AssignToken::desc(){
  return expr->desc() + " = " + value->desc();
}

ParseResult AssignParser::scheme(CC *cc, Source *s, int pos){
  auto tid = ip.parse(cc, s, pos);
  auto ans = tid >> esp >> vp;
  if(!ans) return ans;
  return ParseResult(new AssignToken(
                                     tid.token,
                                     ((TupleToken*) ans.token)->t2,
                                     cc, s,
                                     ans.token->spos,
                                     ans.token->epos));
}

AssignParser::AssignParser()
  : esp(s_eq)
{}
