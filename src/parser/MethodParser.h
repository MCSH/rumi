#pragma once
#include "FunctionParser.h"
#include "parser.h"
#include "IdParser.h"
#include "SymbolParser.h"

class MethodToken: public Token{
public:
  std::string structName;
  std::string methodName;
  Token *f;

  MethodToken(std::string structName, std::string methodName, Token *f, CC *cc, Source *s, int spos, int epos);

  virtual void *get(std::string key) override;
  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class MethodParser: public ParseRule{
 public:

  ParseResult scheme(CC *cc, Source *s, int pos);

  MethodParser();

 private:
  IdParser idp;
  SymbolParser dsp, csp, esp;
  FunctionParser fp;
};
