#pragma once
#include "KeywordParser.h"
#include "TypeParser.h"

class SizeofToken: public Token{
 public:
  Token *type;

  SizeofToken(Token *type, CC *cc, Source *s, int spos, int epos);

  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class SizeofParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  SizeofParser();
 private:
  KeywordParser kp;
  TypeParser tp;
};
