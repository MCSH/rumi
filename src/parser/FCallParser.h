#pragma once
#include "ExpressionParser.h"
#include "IdParser.h"
#include "SymbolParser.h"

class FCallParser: public ParseRule{
 public:
  virtual Token *scheme(CC *cc, Source *s, int pos);
  FCallParser();
 private:
  SymbolParser lpar, rpar, cp;
  IdParser idp;
  ExpressionParser ep;
};
