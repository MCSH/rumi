#include "FunctionTypeParser.h"
#include "Symbols.h"

#include <iostream>

Token *FunctionTypeParser::scheme(CC *cc, Source *s, int pos){
  auto tmp = lparsp.parse(cc, s, pos); // (
  auto bak = tmp;
  while(true){
    tmp = tmp >> tp;
    if(!tmp) {
      tmp = bak;
      break;
    }
    if(!(tmp >> csp)) break;
    tmp = tmp >> csp;
  }

  return tmp >> rparsp >> tosp >> tp; // ) -> tp
}

FunctionTypeParser::FunctionTypeParser()
  : lparsp(s_lpar)
  , rparsp(s_rpar)
  , tosp(s_to)
  , csp(s_comma)
{}
