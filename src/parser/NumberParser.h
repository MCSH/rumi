#pragma once
#include "../CC.h"
#include "parser.h"

class NumberToken: public Token{
public:
  int val;
  NumberToken(CC *cc, Source *s, int spos, int epos, int val);
  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class FloatNumberToken: public Token{
public:
  double val;
  bool is32;
  FloatNumberToken(CC *cc, Source *s, int spos, int epos, double val, bool is32);
  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class NumberParser: public ParseRule{
public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
};
