#pragma once
#include <vector>
#include "Token.h"

class TupleToken: public Token{
public:
  Token *t1, *t2;
  TupleToken(Token *t1, Token *t2);
  ~TupleToken();
  virtual std::string desc() override;
};

class NumberToken: public Token{
public:
  int val;
  NumberToken(CC *cc, Source *s, int spos, int epos, int val);
  virtual std::string desc() override;
};

class ParseRule {
public:
  virtual Token *scheme(CC *cc, Source *s, int pos) = 0;
};

class Parser {
private:
  std::vector<ParseRule *> topRules;
  CC *cc;

public:
  void init(CompileContext *cc);
  void registerTopRule(ParseRule *p);
  Token *parseTop(Source *s, int pos = 0);
};

int skipws(std::string *w, int pos);
int nextws(std::string *w, int pos);

Token *operator>>(Token *p1, ParseRule &p2);

bool isalpha(char c);
bool isnumerical(char c);
bool isalphanumerical(char c);
