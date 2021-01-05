#include "FCallParser.h"
#include "Symbols.h"

ParseResult FCallParser::scheme(CC *cc, Source *s, int pos){
  // id ( )
  auto id = idp.parse(cc, s, pos) >> lpar;
  // arguments
  auto tmp = id >> ep;
  if (!tmp) return id >> rpar; // no arguments

  // id ( v1,
  auto t = tmp >> cp;
  while(t){
    tmp = t >> ep;
    t = tmp >> cp;
  }

  return tmp >> rpar;
}

FCallParser::FCallParser()
  : lpar(s_lpar)
  , rpar(s_rpar)
  , cp(s_comma)
{}
