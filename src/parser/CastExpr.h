#pragma once
#include "../CC.h"
#include "SymbolParser.h"
#include "parser.h"
#include "ExpressionParser.h"
#include "TypeParser.h"

class CastToken: public Token{
 public:
  Token *exp, *type;
  CastToken(Token *exp, Token *type, CC *cc, Source *s, int spos, int epos);

  virtual void *get(std::string key) override;
  virtual std::string desc() override;
  virtual AST *toAST(CC *cC) override;
};

class CastExpr: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  ParseResult innerscheme(CC *cc, Source *s, int pos);
  CastExpr();
private:
  SymbolParser asp;
  ExpressionParser exp;
  TypeParser tp;
};
