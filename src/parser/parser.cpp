#include "parser.h"
#include "../base.h"

#include "KeywordParser.h"
#include "IdParser.h"
#include "NumberParser.h"
#include "SymbolParser.h"
#include <iostream>

void Parser::init(CompileContext *cc){
  this->cc = cc;
  this->registerTopRule(new KeywordParser());
  this->registerTopRule(new NumberParser());
  this->registerTopRule(new IdParser());
  this->registerTopRule(new SymbolParser());
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

Token* Parser::parseTop(Source *s, int pos){
  // TODO
  Token *ans = 0;
  for(auto r: topRules){
    ans = r->scheme(cc, s, pos);
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
  Token * ans = p2.scheme(t->cc, t->s, t->epos);
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
