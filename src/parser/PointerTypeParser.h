#pragma once
#include "parser.h"
#include "SymbolParser.h"
#include "TypeParser.h"

class PointerTypeParser: public ParseRule{
 public:
  virtual Token *scheme(CC *cc, Source *s, int pos);
  PointerTypeParser();
 private:
  SymbolParser ssp;
  TypeParser tp;
};
