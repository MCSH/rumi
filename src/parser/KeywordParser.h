#pragma once
#include "../CC.h"
#include "parser.h"

class KeywordToken : public Token {
public:
  Keyword kw;
  KeywordToken(Keyword kw, int spos, int epos, CC* cc, Source *s)
    : kw(kw) {this->spos = spos; this->epos = epos; this->cc = cc; this->s =s;}
  virtual std::string desc() override;
};

class KeywordParser : public ParseRule {
  bool has_match;
  Keyword match;
  KeywordToken *findkey(CC *cc, Source *s, int pos);
public:
  virtual Token *scheme(CC *cc, Source *s, int pos);
  KeywordParser(Keyword );
  KeywordParser();
};
