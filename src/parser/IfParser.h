#pragma once
#include "ExpressionParser.h"
#include "KeywordParser.h"
#include "SymbolParser.h"
#include "StatementParser.h"

class IfToken: public Token{
public:
  Token *value, *st1, *st2;
  IfToken(Token *value, Token *st1, Token *st2, CC *cc, Source *s, int pos, int epos);
  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class IfParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  IfParser();
 private:
  ExpressionParser vp;
  KeywordParser ip, ep;
  SymbolParser lpar, rpar;
  StatementParser sp;
};
