#pragma once
#include "NumberParser.h"
#include "SymbolParser.h"
#include "parser.h"
#include "KeywordParser.h"
#include "IdParser.h"

class EnumMember{
 public:
  std::string key;
  int num;
  bool hasNum = false;
};

class EnumToken: public Token{
public:
  std::vector<EnumMember *> members;
  std::string id;

  EnumToken(CC *cc, Source *s, int spos, int epos);

  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class EnumParser: public ParseRule{
 public:

  EnumParser();

  ParseResult scheme(CC *cc, Source *s, int pos);

 private:
  IdParser idp;
  KeywordParser ekp;
  SymbolParser csp, comsp;
  SymbolParser lcsp, rcsp;
  SymbolParser eqp;
  NumberParser np;
};
