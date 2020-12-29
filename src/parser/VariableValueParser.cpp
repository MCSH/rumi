#include "VariableValueParser.h"
#include "Symbols.h"

#include <iostream>

Token *VariableValueParser::scheme(CC *cc, Source *s, int pos){
  auto a = vp.parse(cc, s, pos);
  return a >> sp;
}


VariableValueParser::VariableValueParser()
  : sp(s_semicolon)
{}
