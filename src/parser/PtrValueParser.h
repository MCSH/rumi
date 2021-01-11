#pragma once
#include "IdParser.h"
#include "SymbolParser.h"
#include "ValueParser.h"
#include "ExpressionParser.h"

class PtrValueToken: public Token{
 public:
  Token *exp;

  PtrValueToken(Token *exp, CC *cc, Source *s, int spos, int epos);

  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class PtrValueParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  PtrValueParser();
 private:
  ExpressionParser ep;
  SymbolParser ssp;
};
