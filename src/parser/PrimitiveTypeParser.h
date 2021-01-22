#pragma once
#include "parser.h"
#include "../TypeEnum.h"

class TypeToken: public Token{
 public:
  TypeEnum key;
  TypeToken(TypeEnum key, int spos, int epos, CC *cc, Source *s);
  virtual void *get(std::string key) override;
  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class PrimitiveTypeParser : public ParseRule {
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
};
