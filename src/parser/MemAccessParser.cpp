#include "MemAccessParser.h"
#include "Symbols.h"
#include "../Source.h"
#include "../ast/MemAccess.h"

MemAccessToken::MemAccessToken(Token *expression, std::string id, CC *cc,
                               Source *s, int spos, int epos)
  : expression(expression)
  , id(id)
{
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

AST *MemAccessToken::toAST(CC *cc){
  return new MemAccess((Expression *) expression->getAST(cc), id);
}

std::string MemAccessToken::desc(){
  return "(" + expression->desc() + ")." + id;
}

ParseResult MemAccessParser::scheme(CC *cc, Source *s, int pos){
  ParseState *ps = s->resolveState(pos);
  if(ps->hasParser("mem")) return ps->getToken("mem"); // recursive
  ps->setToken("mem", 0);

  Token* tmp =  innerscheme(cc, s, pos).token;

  ps->setToken("mem", tmp);
  return ParseResult(tmp);
}

ParseResult MemAccessParser::innerscheme(CC *cc, Source *s, int pos){
  auto exp = ep.parse(cc, s, pos);
  auto tmp = exp >> dsp >> ip;
  if(!tmp) return tmp;
  while(tmp){
    std::string id = ((IdToken *)((TupleToken *) tmp.token) -> t2)->id;
    exp = new MemAccessToken(exp.token, id, cc, s, tmp.token->spos, tmp.token->epos);
    tmp = tmp >> dsp >> ip;
  }

  return exp;
}

int MemAccessParser::prec(){
  return 3;
}

MemAccessParser::MemAccessParser()
  : dsp(s_dot)
  , ep(5)
{}
