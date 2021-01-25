#pragma once
#include "ExpressionParser.h"
#include "IdParser.h"
#include "SymbolParser.h"
#include "../ast/FCall.h"

class FCallToken: public Token{
public:
  std::string id;
  std::vector<Token *> args;
  FCallToken(std::string id, CC *cc, Source *s, int spos, int epos);
  virtual ~FCallToken(){} // TODO
  virtual std::string desc() override;
  virtual FCall *toAST(CC *cc) override;
};

class FCallParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  FCallParser();
 private:
  SymbolParser lpar, rpar, cp;
  IdParser idp;
  ExpressionParser ep;
};
