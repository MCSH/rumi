#pragma once
#include "DefineParser.h"
#include "SymbolParser.h"
#include "parser.h"
#include "KeywordParser.h"

class InterfaceToken: public Token{
 public:
  std::string id;
  std::vector<DefineToken *> methods;

  InterfaceToken(std::string id, CC *cc, Source *s, int spos, int eops);

  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class InterfaceParser: public ParseRule{
 public:

  InterfaceParser();

  ParseResult scheme(CC *cc, Source *s, int pos);

 private:
  IdParser idp;
  KeywordParser ikp;
  SymbolParser csp;
  SymbolParser lcsp, rcsp;
  DefineParser dp;
};
