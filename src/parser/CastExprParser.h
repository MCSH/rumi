#pragma once
#include "../CC.h"
#include "SymbolParser.h"
#include "parser.h"
#include "ExpressionParser.h"
#include "TypeParser.h"

class CastExprToken: public Token{
 public:
  Token *exp, *type;
  CastExprToken(Token *exp, Token *type, CC *cc, Source *s, int spos, int epos);

  virtual std::string desc() override;
  virtual AST *toAST(CC *cC) override;
};

class CastExprParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  ParseResult innerscheme(CC *cc, Source *s, int pos);
  CastExprParser();
private:
  SymbolParser asp;
  ExpressionParser exp;
  TypeParser tp;
};
