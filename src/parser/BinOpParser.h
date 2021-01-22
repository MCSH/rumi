#pragma once
#include "ExpressionParser.h"
#include "SymbolParser.h"

class BinOpToken: public Token{
public:
  Symbol op;
  Token *lhs, *rhs;
  int prec;
  BinOpToken(Symbol op, Token *lhs, Token *rhs, CC *cc, Source *s, int pos, int epos);
  virtual void *get(std::string key) override;
  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class BinOpParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  ParseResult innerscheme(CC *cc, Source *s, int pos);
  BinOpParser();
 private:
  SymbolParser addp, minusp, multp, divp, remp;
  SymbolParser eqp, neqp;
  SymbolParser gtp, gtep;
  SymbolParser ltp, ltep;
  SymbolParser andp, orp;
  ExpressionParser vp;
};
