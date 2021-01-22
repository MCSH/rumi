#include "TopParser.h"
#include "parser.h"
#include "../base.h"

ParseResult TopParser::scheme(CC *cc, Source *s, int pos){
  return cc->parser.parseTop(s, pos);
}
