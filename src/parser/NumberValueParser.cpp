#include "NumberValueParser.h"
#include "Symbols.h"

Token *NumberValueParser::scheme(CC *cc, Source *s, int pos){
  return np.parse(cc, s, pos) >> sp;
}


NumberValueParser::NumberValueParser()
  : sp(s_semicolon)
{}
