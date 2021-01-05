#pragma once
#include "../CC.h"
#include "parser.h"

class TypeParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
};
