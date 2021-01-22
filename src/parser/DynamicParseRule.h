#pragma once
#include "parser.h"


class DynamicParseRule: public ParseRule{

  ParseResult * (*innerf)(void *p, CC *cc, Source *s, int pos);
  
 public:
  void *p;
  std::string name;

  DynamicParseRule(CC *cc, std::string name);
  
  ParseResult scheme(CC *cc, Source *s, int pos);
};

class DynamicParseToken: public Token{
public:
  DynamicParseRule *drp;
  Token *res;

  DynamicParseToken(DynamicParseRule *drp, Token *res);

  virtual std::string desc() override;
  virtual AST* toAST(CC *cc) override;

  virtual void *get(std::string key) override;
};
