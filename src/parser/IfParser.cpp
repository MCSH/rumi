#include "IfParser.h"
#include "Symbols.h"
#include "Keywords.h"

Token *IfParser::scheme(CC *cc, Source *s, int pos){
  auto i = ip.parse(cc, s, pos) >> lpar >> vp >> rpar >> sp;
  auto tmp = i >> ep >> sp;
  if(tmp) return tmp;
  return i;
}


IfParser::IfParser()
  : lpar(s_lpar)
  , rpar(s_rpar)
  , ip(k_if)
  , ep(k_else)
{}
