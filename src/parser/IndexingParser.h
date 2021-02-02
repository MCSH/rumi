#pragma once
#include "ExpressionParser.h"
#include "SymbolParser.h"

// TODO expose in compile API
class IndexingToken: public Token{
 public:
  Token *expr, *index;

  IndexingToken(Token *expr, Token *index, CC *cc, Source *s, int spos, int epos);

  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class IndexingParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos) override;
  ParseResult innerscheme(CC *cc, Source *s, int pos);
  virtual int prec() override;
  IndexingParser();
 private:
  ExpressionParser vp, ip;
  SymbolParser lbra, rbra;
};
