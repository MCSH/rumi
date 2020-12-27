#pragma once
#include <string>
#include <vector>

class CompileContext;
using CC = CompileContext;
class Source;

class Token {
public:
  virtual std::string desc() = 0;
  int spos;
  int epos;

  // hack to allow operator
  Source *s;
  CC *cc;
};

std::ostream &operator<<(std::ostream &os, Token &s);

enum Keyword {
  k_ret,
  k_if,
  k_while,
};

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

class KeywordToken : public Token {
public:
  Keyword kw;
  KeywordToken(Keyword kw, int spos, int epos, CC* cc, Source *s)
    : kw(kw) {this->spos = spos; this->epos = epos; this->cc = cc; this->s =s;}
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

class KeywordParser : public ParseRule {
public:
  virtual Token *scheme(CC *cc, Source *s, int pos);
};

class CompositeParser: public ParseRule{
private:
  Parser *p1, *p2;
public:
  virtual Token *scheme(CC *cc, Source *s, int pos);
};

class TwoKeywordParser : public ParseRule{
public:
  virtual Token *scheme(CC *cc, Source *s, int pos);
private:
  KeywordParser kps;
};

class NumberParser: public ParseRule{
public:
  virtual Token *scheme(CC *cc, Source *s, int pos);
};

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

int skipws(std::string *w, int pos);
int nextws(std::string *w, int pos);

Token *operator>>(Token *p1, ParseRule &p2);
