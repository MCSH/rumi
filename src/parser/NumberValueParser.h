#pragma once
#include "NumberParser.h"
#include "SymbolParser.h"

class NumberValueParser: public ParseRule{
public:
  virtual Token *scheme(CC *cc, Source *s, int pos);
  NumberValueParser();
private:
  NumberParser np;
  SymbolParser sp;
};
