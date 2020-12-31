#pragma once
#include "ExpressionParser.h"
#include "SymbolParser.h"

class BinOpParser: public ParseRule{
 public:
  virtual Token *scheme(CC *cc, Source *s, int pos);
  Token *innerscheme(CC *cc, Source *s, int pos);
  BinOpParser();
 private:
  SymbolParser addp, minusp, multp, divp, remp;
  ExpressionParser vp;
};
