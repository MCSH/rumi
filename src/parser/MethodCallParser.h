#pragma once
#include "MemAccessParser.h"
#include "parser.h"
#include "SymbolParser.h"
#include "IdParser.h"
#include "ExpressionParser.h"

class MethodCallToken: public Token{
public:
  Token *exp;
  std::vector<Token *> args;

  MethodCallToken(Token *exp, CC *cc, Source *s, int spos, int epos);

  virtual void *get(std::string key) override;
  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class MethodCallParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  MethodCallParser();
 private:
  SymbolParser lpar, rpar, cp;
  MemAccessParser map;
  IdParser idp;
  ExpressionParser ep;
};
