#pragma once
#include "DefineParser.h"
#include "SymbolParser.h"
#include "parser.h"
#include "KeywordParser.h"

class StructToken: public Token{
public:
  std::vector<DefineToken *> members;
  std::string id;

  StructToken(CC *cc, Source *s, int spos, int epos);

  virtual void *get(std::string key) override;
  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class StructParser: public ParseRule{
 public:

  StructParser();

  ParseResult scheme(CC *cc, Source *s, int pos);

 private:
  IdParser idp;
  KeywordParser skp;
  SymbolParser csp;
  SymbolParser lcsp, rcsp;
  DefineParser dp;
};
