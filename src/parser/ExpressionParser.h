#pragma once
#include "../CC.h"
#include "parser.h"

class ExpressionParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
};
