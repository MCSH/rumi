#pragma once
#include "ExpressionParser.h"
#include "KeywordParser.h"
#include "SymbolParser.h"
#include "StatementParser.h"

class IfParser: public ParseRule{
 public:
  virtual Token *scheme(CC *cc, Source *s, int pos);
  IfParser();
 private:
  ExpressionParser vp;
  KeywordParser ip, ep;
  SymbolParser lpar, rpar;
  StatementParser sp;
};
