#pragma once
#include "ExpressionParser.h"
#include "IdParser.h"
#include "SymbolParser.h"
#include "../ast/FCall.h"

class FCallStmtToken: public Token{
public:
  std::string id;
  std::vector<Token *> args;
  FCallStmtToken(std::string id, CC *cc, Source *s, int spos, int epos);
  virtual ~FCallStmtToken(){} // TODO
  virtual void *get(std::string key) override;
  virtual std::string desc() override;
  virtual FCallStmt *toAST(CC *cc) override;
};

class FCallStmtParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
  FCallStmtParser();
 private:
  SymbolParser lpar, rpar, cp, scp;
  IdParser idp;
  ExpressionParser ep;
};
