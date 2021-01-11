#pragma once
#include "../CC.h"
#include "parser.h"

class ExpressionParser: public ParseRule{
 public:
  int prec;

  ExpressionParser();
  ExpressionParser(int prec);
  
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
};
