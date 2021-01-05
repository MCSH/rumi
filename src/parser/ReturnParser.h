#pragma once
#include "ValueParser.h"
#include "KeywordParser.h"
#include "SymbolParser.h"

class ReturnParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  ReturnParser();
 private:
  ValueParser vp;
  KeywordParser rp;
  SymbolParser sp;
};
