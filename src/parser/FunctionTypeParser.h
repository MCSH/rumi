#pragma once
#include "TypeParser.h"
#include "parser.h"
#include "SymbolParser.h"

class FunctionTypeToken: public Token{
public:
  std::vector<Token*> args;
  Token *rt = 0;
  FunctionTypeToken(CC *cc, Source *s, int pos, int epos);
  virtual std::string desc() override;
};

class FunctionTypeParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  FunctionTypeParser();
 private:
  SymbolParser lparsp, rparsp;
  SymbolParser tosp, csp;
  TypeParser tp;
};
