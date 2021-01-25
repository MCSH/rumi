#pragma once
#include "SymbolParser.h"
#include "StatementParser.h"

class SBlockToken: public Token{
public:
  std::vector<Token *> stmts;

  SBlockToken(CC *cc, Source *s, int spos, int epos);
  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class SBlockParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  SBlockParser();
 private:
  SymbolParser lbra, rbra;
  StatementParser sp;
};
