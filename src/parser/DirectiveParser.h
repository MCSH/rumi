#pragma once
#include "parser.h"
#include "IdParser.h"
#include "SymbolParser.h"

class DirectiveToken: public Token{
public:
  std::string id;
  Token *top;
  
  DirectiveToken(std::string id, Token *top, CC *cc, Source *s, int spos, int epos);

  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class DirectiveParser: public ParseRule{
public:
  ParseResult scheme(CC *cc, Source *s, int pos);

  DirectiveParser();

private:
  IdParser idp;
  SymbolParser atp;
};
