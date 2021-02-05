#pragma once
#include "parser.h"
#include "KeywordParser.h"
#include "ExpressionParser.h"
#include "SymbolParser.h"

// TODO expose this to the compiler API

class UsingToken: public Token{
 public:
  Token *exp;

  UsingToken(Token *exp, CC *cc, Source *s, int spos, int epos);

  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class UsingParser: public ParseRule{
 public:
  ParseResult scheme(CC *cc, Source *s, int pos);
  UsingParser();
 private:
  KeywordParser ukp;
  ExpressionParser ep;
  SymbolParser sp;
};
