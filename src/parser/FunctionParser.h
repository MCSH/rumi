#pragma once
#include "../CC.h"
#include "IdParser.h"
#include "KeywordParser.h"
#include "SymbolParser.h"
#include "TypeParser.h"
#include "parser.h"
#include "Symbols.h"
#include "StatementParser.h"
#include "../ast/Function.h"

class FunctionBodyToken: public Token{
 public:
  std::vector<Token*> statements;
  std::vector<Token*> args;
  Token *rt = 0;
  FunctionBodyToken(CC *cc, Source *s, int pos, int epos);
  virtual std::string desc() override;
  virtual Function *toAST(CC *cc) override;
};

class FunctionParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  FunctionParser();
private:
  SymbolParser lpsp, rpsp;
  SymbolParser lbsp, rbsp;
  SymbolParser asp, csp;
  SymbolParser comsp;
  TypeParser tp;
  IdParser ip;
  StatementParser sp;
};
