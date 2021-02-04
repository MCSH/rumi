#pragma once
#include "parser.h"
#include "SymbolParser.h"

// Currently only supports binops
// TODO: Use this in preop as well
// TODO: Support user defined tokens here somehow
// TODO: Provide in compiler API
class OpParser: public ParseRule{
 public:
  ParseResult scheme(CC *cc, Source *s, int pos);
  OpParser();
 private:
  SymbolParser addp, minusp, multp, divp, remp;
  SymbolParser eqp, neqp;
  SymbolParser gtp, gtep;
  SymbolParser ltp, ltep;
  SymbolParser andp, orp;
  SymbolParser carP, pipP, ampP;
  SymbolParser lshiftP, rshiftP;
};
