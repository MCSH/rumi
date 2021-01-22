#pragma once
#include "../CC.h"
#include "DefineParser.h"
#include "IdParser.h"
#include "SymbolParser.h"
#include "TypeParser.h"
#include "parser.h"
#include "Symbols.h"
#include "../ast/FunctionSig.h"

class FunctionSigToken: public Token{
 public:
  std::vector<DefineToken*> args;
  Token *returnType;

  FunctionSigToken(CC *cc, Source *s, int spos, int epos);

  virtual void *get(std::string key) override;
  virtual std::string desc() override;
  virtual FunctionSig* toAST(CC *cc);

  bool vararg = false;
};

class FunctionSigParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  FunctionSigParser();
 private:
  SymbolParser lpsp, rpsp;
  SymbolParser asp, csp;
  SymbolParser comsp, ssp;
  SymbolParser tripledotsp;
  TypeParser tp;
  IdParser ip;
};
