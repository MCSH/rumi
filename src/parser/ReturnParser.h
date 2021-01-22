#pragma once
#include "ValueParser.h"
#include "KeywordParser.h"
#include "SymbolParser.h"

class ReturnToken: public Token{
public:
  Token *value;
  ReturnToken(Token *value, CC *cc, Source *s, int spos, int epos);
  virtual void *get(std::string key) override;
  virtual std::string desc() override;
  virtual AST* toAST(CC *cc) override;
};

class ReturnParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  ReturnParser();
 private:
  ValueParser vp;
  KeywordParser rp;
  SymbolParser sp;
};
