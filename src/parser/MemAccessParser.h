#pragma once
#include "parser.h"
#include "SymbolParser.h"
#include "IdParser.h"
#include "ExpressionParser.h"

class MemAccessToken: public Token{
public:
  Token *expression;
  std::string id;

  MemAccessToken(Token *expression, std::string id, CC *cc, Source *s, int spos, int epos);

  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class MemAccessParser: public ParseRule{
 public:
  ParseResult scheme(CC *cc, Source *s, int pos) override;
  ParseResult innerscheme(CC *cc, Source *s, int pos);

  virtual int prec() override;
  MemAccessParser();
private:
  ExpressionParser ep;
  IdParser ip;
  SymbolParser dsp;
};
