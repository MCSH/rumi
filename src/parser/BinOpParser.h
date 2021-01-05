#pragma once
#include "ExpressionParser.h"
#include "SymbolParser.h"

class BinOpParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  ParseResult innerscheme(CC *cc, Source *s, int pos);
  BinOpParser();
 private:
  SymbolParser addp, minusp, multp, divp, remp;
  ExpressionParser vp;
};
