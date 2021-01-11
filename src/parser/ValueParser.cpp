#include "ValueParser.h"
#include "parser.h"
#include "../base.h"
#include "Symbols.h"

ParseResult ValueParser::scheme(CC *cc, Source *s, int pos){
  auto a = cc->parser.parseValue(s, pos);
  if(a) return a;
  a = cc->parser.parseExpression(s, pos, -1) >> sp;
  if(!a) return a;
  Token *f = ((TupleToken*)a.token)->t1;
  f->epos = a.token->epos;
  return ParseResult(f);
}

ValueParser::ValueParser()
  :sp(s_semicolon)
{}
