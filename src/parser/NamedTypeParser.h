#pragma once
#include "parser.h"
#include "IdParser.h"

class NamedTypeToken: public Token{
 public:
  std::string typeId;
  NamedTypeToken(std::string typeId, CC *cc, Source *s, int spos, int epos);
  virtual void *get(std::string key) override;
  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class NamedTypeParser : public ParseRule {
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
 private:
  IdParser idp;
};
