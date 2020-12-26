#include "parser.h"
#include "../base.h"

#include <iostream>

void Parser::init(CompileContext *cc){
  this->cc = cc;
  this->registerTopRule(new TwoKeywordParser());
  this->registerTopRule(new KeywordParser());
}

Token* KeywordParser::scheme(CC *cc, Source *s, int pos){
  // TODO
  // s->wordAt(pos);
  pos = skipws(&s->str, pos);
  if(pos == -1) return 0;
  int end = nextws(&s->str, pos);
  std::string w = s->str.substr(pos, end - pos);
  if(w == "return") return new KeywordToken(Keyword::k_ret, pos, end, cc, s);
  return 0;
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
    if(pos == len){
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

std::string KeywordToken::desc(){
  switch(kw){
  case k_ret:
    return "return";
  default:
    return "[Unknown keyword]";
  }
}


Token *TwoKeywordParser::scheme(CC *cc, Source *s, int pos){
  return kps.scheme(cc, s, pos) >> kps;
}

Token *operator>>(Token *t, ParseRule &p2){
  if(!t) return t;
  return p2.scheme(t->cc, t->s, t->epos);
}
