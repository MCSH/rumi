#pragma once
#include "SymbolParser.h"
#include "StatementParser.h"

class SBlockParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  SBlockParser();
 private:
  SymbolParser lbra, rbra;
  StatementParser sp;
};
