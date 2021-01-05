#pragma once
#include "../CC.h"
#include "IdParser.h"
#include "SymbolParser.h"
#include "parser.h"
#include "Symbols.h"
#include "TypeParser.h"
#include "ValueParser.h"

class DefineToken: public Token{
 public:
  std::string id;
  Token *type, *value;
  DefineToken(std::string id, Token *type, Token* value, CC *cc, Source *s, int pos, int epos);
  virtual std::string desc() override;
};

class DefineParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  DefineParser();
private:
  SymbolParser csp, esp, scp;
  TypeParser tp;
  ValueParser vp;
  IdParser ip;
};
