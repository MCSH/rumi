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

std::string symbolDesc(Symbol s);

class SymbolParser: public ParseRule{
private:
  bool sb_set;
  Symbol sb;
public:
  SymbolParser():sb_set(false){}
  SymbolParser(Symbol sb):sb_set(true), sb(sb) {}
  virtual ParseResult scheme(CC *cc, Source *s, int pos);
};
