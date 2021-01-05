#include "TypeParser.h"
#include "parser.h"
#include "../base.h"

ParseResult TypeParser::scheme(CC *cc, Source *s, int pos){
  return cc->parser.parseType(s, pos);
}
