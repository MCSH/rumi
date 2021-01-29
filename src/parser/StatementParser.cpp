#include "StatementParser.h"
#include "parser.h"
#include "../base.h"

ParseResult StatementParser::scheme(CC *cc, Source *s, int pos){
  return cc->parser.parseStatement(s, pos);
}
