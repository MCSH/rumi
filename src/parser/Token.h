#pragma once
#include "../CC.h"

class AST;

class Token {
public:
  virtual std::string desc() = 0;
  int spos;
  int epos;

  // hack to allow operator
  Source *s;
  CC *cc;

  virtual AST* toAST(CC *cc){return 0;}

  virtual ~Token() = 0;

  virtual void *get(std::string key) = 0;
};

std::ostream &operator<<(std::ostream &os, Token &s);

class ParseResult{
public:
  Token *token;
  ParseResult(Token *t):token(t){}
  ParseResult():token(0){}
  ~ParseResult();
  operator bool() const{
    return this->token;
  }
};

std::ostream &operator<<(std::ostream &os, ParseResult &s);
