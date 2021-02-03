#pragma once
#include "SymbolParser.h"
#include "ExpressionParser.h"

class ParenToken: public Token{
 public:
  Token *exp;

  ParenToken(Token *exp, CC *cc, Source *s, int spos, int epos);

  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class ParenParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos) override;
  ParenParser();
  virtual int prec() override;
 private:
  ExpressionParser ep;
  SymbolParser lpsp, rpsp;
};
