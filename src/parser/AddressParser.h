#pragma once
#include "parser.h"
#include "SymbolParser.h"
#include "ExpressionParser.h"

class AddressToken: public Token{
public:
  Token *exp;

  AddressToken(Token *exp, CC *cc, Source *s, int spos, int epos);

  virtual void *get(std::string key) override;

  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class AddressParser: public ParseRule{
public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  ParseResult innerscheme(CC *cc, Source *s, int pos);
  AddressParser();
private:
  SymbolParser sp;
  ExpressionParser ep;
};
