#include "ExpressionParser.h"
#include "parser.h"
#include "../base.h"

ParseResult ExpressionParser::scheme(CC *cc, Source *s, int pos){
  return cc->parser.parseExpression(s, pos);
}
