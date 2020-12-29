#include "ValueParser.h"
#include "parser.h"
#include "../base.h"
#include "Symbols.h"

Token *ValueParser::scheme(CC *cc, Source *s, int pos){
  auto a = cc->parser.parseValue(s, pos);
  if(a) return a;
  return cc->parser.parseExpression(s, pos) >> sp;
}

ValueParser::ValueParser()
  :sp(s_semicolon)
{}
