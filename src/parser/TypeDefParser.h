#pragma once
#include "../CC.h"
#include "SymbolParser.h"
#include "parser.h"
#include "Symbols.h"
#include "TypeParser.h"
#include "IdParser.h"
#include "KeywordParser.h"

class TypeDefToken: public Token{
public:
  std::string id;
  Token *type;

  TypeDefToken(std::string id, Token *type, CC *cc, Source *s, int pos, int epos);
  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class TypeDefParser: public ParseRule{
public:
  TypeDefParser();
  
  ParseResult scheme(CC *cc, Source *s, int pos);
private:
  IdParser idp;
  KeywordParser tkp; // type keyword parser
  SymbolParser csp, esp, scp; // : and = ;
  TypeParser tp;
};
