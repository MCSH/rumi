#pragma once
#include "parser.h"
#include "KeywordParser.h"

class BoolToken: public Token{
public:
  bool truth;
  BoolToken(CC *cc, Source *s, int spos, int epos, bool truth);

  virtual void *get(std::string key) override;
  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class BoolValueParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  BoolValueParser();

 private:
  KeywordParser tp, fp;
};

