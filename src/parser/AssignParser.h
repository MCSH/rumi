#pragma once
#include "ExpressionParser.h"
#include "SymbolParser.h"
#include "ValueParser.h"

class AssignToken: public Token{
public:
  Token *expr, *value;
  AssignToken(Token *expr, Token *value, CC *cc, Source *s, int pos, int epos);
  virtual void *get(std::string key) override;
  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class AssignParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  AssignParser();
 private:
  ExpressionParser ip;
  ValueParser vp;
  SymbolParser esp;
};
