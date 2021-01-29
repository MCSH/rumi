#pragma once
#include "../CC.h"

class AST;
class DebugInfo;

class Token {
  virtual AST* toAST(CC *cc){return 0;}
public:
  virtual std::string desc() = 0;
  int spos;
  int epos;

  // hack to allow operator
  Source *s;
  CC *cc;

  DebugInfo *getDBG();

  AST *getAST(CC *cc);
  virtual ~Token() = 0;
};

std::ostream &operator<<(std::ostream &os, Token &s);

class Failure{
public:
  int pos;
  Source *s;
  CC *cc;
};

class ParseResult{
public:
  Token *token;
  Failure *failure;
  ParseResult(Token *t):token(t), failure(0){}
  ParseResult(Failure *f):token(0), failure(f){}
  ParseResult():token(0),failure(0){}
  ~ParseResult();
  operator bool() const{
    return this->token;
  }
};

std::ostream &operator<<(std::ostream &os, ParseResult &s);
