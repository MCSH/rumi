#include "parser.h"
#include "../base.h"

#include <iostream>

void Parser::init(CompileContext *cc){
  this->cc = cc;
  this->registerTopRule(new TwoKeywordParser());
  this->registerTopRule(new KeywordParser());
  this->registerTopRule(new NumberParser());
  this->registerTopRule(new IdParser());
  this->registerTopRule(new SymbolParser());
}

Token* KeywordParser::scheme(CC *cc, Source *s, int pos){
  // TODO
  // s->wordAt(pos);
  pos = skipws(&s->str, pos);
  if(pos == -1) return 0;
  int end = nextws(&s->str, pos);
  std::string w = s->str.substr(pos, end - pos);
  if(w == "return") return new KeywordToken(Keyword::k_ret, pos, end, cc, s);
  if(w == "if") return new KeywordToken(Keyword::k_if, pos, end, cc, s);
  if(w == "while") return new KeywordToken(Keyword::k_while, pos, end, cc, s);
  return 0;
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

Token *IdParser::scheme(CC *cc, Source *s, int pos){
  if(kp.scheme(cc, s, pos)) return 0;
  pos = skipws(&s->str, pos);
  if(pos == -1) return 0;
  int end = pos;
  int len = s->str.size();
  char c = s->str.at(pos);
  if(!isalpha(c)) return 0;
  do{
    end++;
    if(end >= len){
      break;
    }
    c = s->str.at(end);
  } while(isalphanumerical(c));
  std::string w = s->str.substr(pos, end - pos);
  return new IdToken(cc, s, pos, end, w);
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

std::string KeywordToken::desc(){
  switch(kw){
  case k_ret:
    return "return";
  case k_if:
    return "if";
  case k_while:
    return "if";
  default:
    return "[Unknown keyword]";
  }
}


Token *TwoKeywordParser::scheme(CC *cc, Source *s, int pos){
  return kps.scheme(cc, s, pos) >> kps;
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

Token *NumberParser::scheme(CC *cc, Source *s, int pos){
  int p = skipws(&s->str, pos);
  if(p == -1) return 0;
  char n = s->str.at(p);
  if(!isnumerical(n)) return 0;
  int val = 0;
  while(isnumerical(n)){
    val *= 10;
    val += n - '0';
    p++;
    n = s->str.at(p);
  }

  auto ans = new NumberToken(cc, s, pos, p, val);
  return ans;
}

std::string NumberToken::desc(){
  return std::to_string(val);
}

NumberToken::NumberToken(CC *cc, Source *s, int spos, int epos, int val){
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
  this->val = val;
}

IdToken::IdToken(CC *cc, Source *s, int spos, int epos, std::string val){
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
  this->id = val;
}

std::string IdToken::desc(){
  return "Id<" + id + ">";
}

Token *SymbolParser::scheme(CC *cc, Source *s, int pos){
  pos = skipws(&s->str, pos);
  if(pos == -1) return 0;
  char c = s->str.at(pos);
  int epos = pos + 1;
  Symbol sb;
  switch(c){
  case '+':
    sb = s_plus;
    break;
  case ';':
    sb = s_semicolon;
    break;
  default:
    return 0;
  }
  return new SymbolToken(sb, pos, epos, cc, s);
}

std::string SymbolToken::desc(){
  switch(sb){
  case s_plus:
    return "<Symbol +>";
  case s_semicolon:
    return "<Symbol ;>";
  default:
    return "<Unprogrammed Symbol>";
  }
}
