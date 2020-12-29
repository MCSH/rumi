#include "ValueParser.h"
#include "parser.h"
#include "../base.h"

Token *ValueParser::scheme(CC *cc, Source *s, int pos){
  return cc->parser.parseValue(s, pos);
  return 0;
}
