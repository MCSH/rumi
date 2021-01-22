#pragma once
#include "../CC.h"
#include "parser.h"

class StringToken: public Token{
 public:
  std::string value;

  StringToken(std::string value, CC *cc, Source *s, int spos, int epos);

  virtual void *get(std::string key) override;
  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class StringParser: public ParseRule{
public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
};
