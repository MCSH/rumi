#include "ExpressionParser.h"
#include "parser.h"
#include "../base.h"

ExpressionParser::ExpressionParser(): prec(-1){}
ExpressionParser::ExpressionParser(int prec): prec(prec){}

ParseResult ExpressionParser::scheme(CC *cc, Source *s, int pos){
  return cc->parser.parseExpression(s, pos, prec);
}
