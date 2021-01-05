#pragma once
#include <vector>
#include "Token.h"

class TupleToken: public Token{
public:
  Token *t1, *t2;
  TupleToken(Token *t1, Token *t2);
  virtual ~TupleToken();
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
  virtual ParseResult scheme(CC *cc, Source *s, int pos) = 0;
  ParseResult parse(CC *cc, Source *s, int pos);
};

class Parser {
private:
  std::vector<ParseRule *> topRules;
  std::vector<ParseRule *> typeRules;
  std::vector<ParseRule *> expressionRules;
  std::vector<ParseRule *> valueRules;
  std::vector<ParseRule *> statementRules;
  CC *cc;

public:
  void init(CompileContext *cc);
  void registerTopRule(ParseRule *p);
  void registerTypeRule(ParseRule *p);
  void registerExpressionRule(ParseRule *p);
  void registerValueRule(ParseRule *p);
  void registerStatementRule(ParseRule *p);
  ParseResult parseTop(Source *s, int pos = 0);
  ParseResult parseType(Source *s, int pos = 0);
  ParseResult parseExpression(Source *s, int pos = 0);
  ParseResult parseValue(Source *s, int pos = 0);
  ParseResult parseStatement(Source *s, int pos = 0);
};

int skipws(std::string *w, int pos);
int skipwscomment(std::string *w, int pos);
int nextws(std::string *w, int pos);

ParseResult operator>>(ParseResult p1, ParseRule &p2);

bool isalpha(char c);
bool isnumerical(char c);
bool isalphanumerical(char c);
int extractNextAlphaNumerical(std::string *str, int pos); // Returns the end of next alpha numerical that starts from pos
