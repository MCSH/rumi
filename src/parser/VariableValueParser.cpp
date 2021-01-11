#include "VariableValueParser.h"
#include "Symbols.h"

#include <iostream>

ParseResult VariableValueParser::scheme(CC *cc, Source *s, int pos){
  return vp.parse(cc, s, pos);
}

int VariableValueParser::prec(){
  return 5;
}
