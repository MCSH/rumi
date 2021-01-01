#pragma once
#include "IdParser.h"
#include "SymbolParser.h"
#include "ValueParser.h"


class AssignParser: public ParseRule{
 public:
  virtual Token *scheme(CC *cc, Source *s, int pos);
  AssignParser();
 private:
  IdParser ip;
  ValueParser vp;
  SymbolParser esp;
};
