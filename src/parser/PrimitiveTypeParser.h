#pragma once
#include "parser.h"

class TypeToken: public Token{
 public:
  std::string key;
  TypeToken(std::string key, int spos, int epos, CC *cc, Source *s);
  virtual std::string desc() override;
};

class PrimitiveTypeParser : public ParseRule {
 public:
  virtual Token *scheme(CC *cc, Source *s, int pos);
};
