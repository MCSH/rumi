#include "TypeParser.h"
#include "parser.h"
#include "../base.h"

Token *TypeParser::scheme(CC *cc, Source *s, int pos){
  return cc->parser.parseType(s, pos);
}
