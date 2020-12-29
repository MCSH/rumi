#include "WhileParser.h"
#include "Symbols.h"
#include "Keywords.h"

Token *WhileParser::scheme(CC *cc, Source *s, int pos){
  return wp.parse(cc, s, pos) >> lpar >> vp >> rpar >> sp;
}


WhileParser::WhileParser()
  : lpar(s_lpar)
  , rpar(s_rpar)
  , wp(k_while)
{}
