#pragma once
#include "../CC.h"
#include "parser.h"
class SymbolToken : public Token {
public:
  Symbol sb;
  SymbolToken(Symbol sb, int spos, int epos, CC* cc, Source *s)
    : sb(sb) {this->spos = spos; this->epos = epos; this->cc = cc; this->s =s;}
  virtual std::string desc() override;
};

class SymbolParser: public ParseRule{
public:
  virtual Token *scheme(CC *cc, Source *s, int pos);
};
