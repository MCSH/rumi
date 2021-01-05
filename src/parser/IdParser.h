#pragma once
#include "../CC.h"
#include "parser.h"
#include "KeywordParser.h"

class IdToken: public Token{
public:
  std::string id;
  IdToken(CC *cc, Source *s, int spos, int epos, std::string val);
  virtual std::string desc() override;
  virtual ~IdToken();
};

class IdParser: public ParseRule{
public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
private:
  KeywordParser kp;
};
