#pragma once
#include "IdParser.h"
#include "SymbolParser.h"

class VariableValueParser: public ParseRule{
public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos) override;

  virtual int prec() override;
private:
  IdParser vp;
};
