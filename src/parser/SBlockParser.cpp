#include "SBlockParser.h"
#include "Symbols.h"
#include "Keywords.h"

ParseResult SBlockParser::scheme(CC *cc, Source *s, int pos){
  auto p = lbra.parse(cc, s, pos);
  auto tmp = p >> sp;
  while(tmp){
    p = tmp;
    tmp = p >> sp;
  }
  return p >> rbra;
}


SBlockParser::SBlockParser()
  : lbra(s_lbra)
  , rbra(s_rbra)
{}
