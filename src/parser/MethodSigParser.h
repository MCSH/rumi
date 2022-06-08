#pragma once
#include "FunctionSigParser.h"
#include "parser.h"
#include "IdParser.h"
#include "SymbolParser.h"
#include "MethodParser.h"  // for MethodToken

class MethodSigToken: public Token{
public:
  std::string structName;
  std::string methodName;
  Token *f;

  MethodSigToken(std::string structName, std::string methodName, Token *f, CC *cc, Source *s, int spos, int epos);

  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class MethodSigParser: public ParseRule{
 public:

  ParseResult scheme(CC *cc, Source *s, int pos);

  MethodSigParser();

 private:
  IdParser idp;
  SymbolParser dsp, csp, esp;
  FunctionSigParser fsp;
};
