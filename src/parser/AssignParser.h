#pragma once
#include "IdParser.h"
#include "SymbolParser.h"
#include "ValueParser.h"

class AssignToken: public Token{
public:
  std::string id;
  Token *value;
  AssignToken(std::string id, Token *value, CC *cc, Source *s, int pos, int epos);
  virtual std::string desc() override;
};

class AssignParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  AssignParser();
 private:
  IdParser ip;
  ValueParser vp;
  SymbolParser esp;
};
