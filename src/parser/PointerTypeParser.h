#pragma once
#include "parser.h"
#include "SymbolParser.h"
#include "TypeParser.h"

class PointerTypeToken: public Token{
public:
  Token *innerType;
  PointerTypeToken(Token *innerType, CC *cc, Source *s, int pos, int epos);
  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class PointerTypeParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  PointerTypeParser();
 private:
  SymbolParser ssp;
  TypeParser tp;
};
