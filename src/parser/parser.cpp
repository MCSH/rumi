#include "parser.h"
#include "../base.h"

#include "DefineParser.h"
#include "KeywordParser.h"
#include "IdParser.h"
#include "NumberValueParser.h"
#include "SymbolParser.h"
#include "FunctionParser.h"
#include "ReturnParser.h"
#include <iostream>

void Parser::init(CompileContext *cc){
  this->cc = cc;
  this->registerTopRule(new DefineParser());

  this->registerValueRule(new NumberValueParser());
  this->registerValueRule(new FunctionParser());

  this->registerStatementRule(new DefineParser());
  this->registerStatementRule(new ReturnParser());
  /*
  this->registerTopRule(new FunctionParser());
  this->registerTopRule(new KeywordParser());
  this->registerTopRule(new NumberParser());
  this->registerTopRule(new IdParser());
  this->registerTopRule(new SymbolParser());
  */
}

bool isalpha(char c){
  return c == '_' || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool isnumerical(char c){
  return c >= '0' && c <= '9';
}

bool isalphanumerical(char c){
  return isalpha(c) || isnumerical(c);
}


bool isws(char c){
  return c == ' ' || c == '\t' || c == '\n';
}

int skipws(std::string *w, int pos){
  pos --;
  int len = w->size();
  char c;
  do{
    pos++;
    if(pos >= len){
      /// TODO maybe there is more?
      return -1;
    }
    c = w->at(pos);
  } while(isws(c));
  return pos;
}

int nextws(std::string *w, int pos){
  pos --;
  int len = w->size();
  char c;
  do{
    pos++;
    if(pos == len){
      /// TODO maybe there is more?
      return pos;
    }
    c = w->at(pos);
  } while(!isws(c));
  return pos;
}

void Parser::registerTopRule(ParseRule *p){
  topRules.push_back(p);
}

void Parser::registerTypeRule(ParseRule *p){
  typeRules.push_back(p);
}

void Parser::registerValueRule(ParseRule *p){
  valueRules.push_back(p);
}

void Parser::registerStatementRule(ParseRule *p){
  statementRules.push_back(p);
}

Token* Parser::parseTop(Source *s, int pos){
  // TODO
  Token *ans = 0;
  for(auto r: topRules){
    ans = r->parse(cc, s, pos);
    if(ans) return ans;
  }
  return 0;
}

Token* Parser::parseType(Source *s, int pos){
  // TODO
  Token *ans = 0;
  for(auto r: typeRules){
    ans = r->parse(cc, s, pos);
    if(ans) return ans;
  }
  return 0;
}

Token* Parser::parseValue(Source *s, int pos){
  // TODO
  Token *ans = 0;
  for(auto r: valueRules){
    ans = r->parse(cc, s, pos);
    if(ans) return ans;
  }
  return 0;
}

Token* Parser::parseStatement(Source *s, int pos){
  // TODO
  Token *ans = 0;
  for(auto r: statementRules){
    ans = r->parse(cc, s, pos);
    if(ans) return ans;
  }
  return 0;
}

std::ostream &operator<<(std::ostream &os, Token &s) {
  os << "[" << s.spos << ", " << s.epos << "]" << s.desc();
  return os;
}


Token *operator>>(Token *t, ParseRule &p2){
  if(!t) return t;
  Token * ans = p2.parse(t->cc, t->s, t->epos + 1);
  if(!ans) return ans;
  return new TupleToken(t, ans);
}


TupleToken::TupleToken(Token *t1, Token *t2):t1(t1),t2(t2){
  this->spos = t1->spos;
  this->epos = t2->epos;
  this->s = t1->s;
  this->cc = t1->cc;
}

TupleToken::~TupleToken(){
  delete t1;
  delete t2;
}

std::string TupleToken::desc(){
  return t1->desc() + " >> " + t2->desc();
}

Token *ParseRule::parse(CC *cc, Source *s, int pos){
  // TODO memoization
  return this->scheme(cc, s, pos);
}
