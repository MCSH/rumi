#pragma once
#include "MethodParser.h"
#include "SymbolParser.h"

class NewParser: public ParseRule{
 public:

  ParseResult scheme(CC *cc, Source *s, int pos);

  NewParser();

 private:
  IdParser idp;
  SymbolParser csp, esp;
  FunctionParser fp;
  KeywordParser kw;
};
