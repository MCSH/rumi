#pragma once
#include "IdParser.h"
#include "SymbolParser.h"

class VariableValueParser: public ParseRule{
public:
  virtual Token *scheme(CC *cc, Source *s, int pos);
  VariableValueParser();
private:
  IdParser vp;
  SymbolParser sp;
};
