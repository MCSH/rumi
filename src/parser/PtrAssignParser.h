#pragma once
#include "IdParser.h"
#include "SymbolParser.h"
#include "ValueParser.h"
#include "ExpressionParser.h"

class PtrAssignToken: public Token{
public:
  Token *ptr, *value;

  PtrAssignToken(Token *ptr, Token *value, CC *cc, Source *s, int spos, int epos);

  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class PtrAssignParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  PtrAssignParser();
 private:
  ValueParser vp;
  ExpressionParser ep;
  SymbolParser esp, ssp;
};
