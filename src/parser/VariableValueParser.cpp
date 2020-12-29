#include "VariableValueParser.h"
#include "Symbols.h"

#include <iostream>

Token *VariableValueParser::scheme(CC *cc, Source *s, int pos){
  return vp.parse(cc, s, pos);
}
