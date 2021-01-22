#pragma once
#include "../CC.h"
#include "parser.h"

class NumberToken: public Token{
public:
  int val;
  NumberToken(CC *cc, Source *s, int spos, int epos, int val);
  virtual void *get(std::string key) override;
  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class NumberParser: public ParseRule{
public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
};
