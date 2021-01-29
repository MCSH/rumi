#include "WhileParser.h"
#include "Symbols.h"
#include "Keywords.h"
#include "../ast/While.h"

WhileToken::WhileToken(Token *value, Token *statement, CC *cc, Source *s, int spos, int epos)
  : value(value)
  , statement(statement)
{
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

AST *WhileToken::toAST(CC *cc){
  return new While((Expression *) value->getAST(cc), (Statement*) statement->getAST(cc));
}

std::string WhileToken::desc(){
  return "while " + value->desc() + ":: " + statement->desc();
}

ParseResult WhileParser::scheme(CC *cc, Source *s, int pos){
  auto a = wp.parse(cc, s, pos) >> lpar >> vp;
  auto b = a >> rpar >> sp;
  if(!b) return b;

  Token *v = ((TupleToken*) a.token)->t2;
  Token *st = ((TupleToken*) b.token)->t2;

  return ParseResult(new WhileToken(v, st, cc, s, b.token->spos, b.token->epos));
}


WhileParser::WhileParser()
  : lpar(s_lpar)
  , rpar(s_rpar)
  , wp(k_while)
{}
