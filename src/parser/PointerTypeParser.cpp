#include "PointerTypeParser.h"
#include "Symbols.h"

ParseResult PointerTypeParser::scheme(CC *cc, Source *s, int pos){
  return ssp.parse(cc, s, pos) >> tp;
}

PointerTypeParser::PointerTypeParser()
  : ssp(s_mult)
{}
