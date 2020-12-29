#include "ReturnParser.h"
#include "Symbols.h"
#include "Keywords.h"

Token *ReturnParser::scheme(CC *cc, Source *s, int pos){
  auto r = rp.parse(cc, s, pos);
  auto tmp = r >> vp;
  if(tmp) return tmp; // semicolon is handled inside that
  return tmp >> sp; // if no value, check for semicolon
}


ReturnParser::ReturnParser()
  : sp(s_semicolon)
  , rp(k_ret)
{}
