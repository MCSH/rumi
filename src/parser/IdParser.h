#pragma once
#include "../CC.h"
#include "parser.h"
#include "KeywordParser.h"

class IdToken: public Token{
public:
  std::string id;
  IdToken(CC *cc, Source *s, int spos, int epos, std::string val);
  virtual std::string desc() override;
};

class IdParser: public ParseRule{
public:
  virtual Token *scheme(CC *cc, Source *s, int pos);
private:
  KeywordParser kp;
};
