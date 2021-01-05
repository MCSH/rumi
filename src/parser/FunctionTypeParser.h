#pragma once
#include "TypeParser.h"
#include "parser.h"
#include "SymbolParser.h"

class FunctionTypeParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  FunctionTypeParser();
 private:
  SymbolParser lparsp, rparsp;
  SymbolParser tosp, csp;
  TypeParser tp;
};
