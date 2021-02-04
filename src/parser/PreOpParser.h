#pragma once
#include "ExpressionParser.h"
#include "SymbolParser.h"

class PreOpToken: public Token{
public:
  std::string op;
  Token *value;

  PreOpToken(CC *cc, Source *s, int spos, int epos, std::string op, Token *value);

  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class PreOpParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);

  PreOpParser();
 private:
  SymbolParser excp;
  SymbolParser tildP;
  ExpressionParser vp;
};
