#pragma once
#include "../CC.h"
#include "IdParser.h"
#include "KeywordParser.h"
#include "SymbolParser.h"
#include "TypeParser.h"
#include "parser.h"
#include "Symbols.h"
#include "StatementParser.h"

class FunctionToken: public Token{
 public:
  FunctionToken(CC *cc, Source *s, int pos, int epos);
  virtual std::string desc() override;
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
