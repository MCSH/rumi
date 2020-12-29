#include "StatementParser.h"
#include "parser.h"
#include "../base.h"

Token *StatementParser::scheme(CC *cc, Source *s, int pos){
  return cc->parser.parseStatement(s, pos);
  return 0;
}
