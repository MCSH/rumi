#pragma once
#include "ExpressionParser.h"
#include "KeywordParser.h"
#include "SymbolParser.h"
#include "StatementParser.h"

class WhileToken: public Token{
public:
  Token *value, *statement;
  WhileToken(Token *value, Token *statement, CC *cc, Source *s, int pos, int epos);
  virtual std::string desc() override;
};

class WhileParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  WhileParser();
 private:
  ExpressionParser vp;
  KeywordParser wp;
  SymbolParser lpar, rpar;
  StatementParser sp;
};
