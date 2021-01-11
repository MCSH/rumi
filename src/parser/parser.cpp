#include "parser.h"
#include "../Source.h"

#include "AssignParser.h"
#include "DefineParser.h"
#include "FunctionTypeParser.h"
#include "IfParser.h"
#include "PointerTypeParser.h"
#include "PrimitiveTypeParser.h"
#include "SBlockParser.h"
#include "WhileParser.h"
#include "KeywordParser.h"
#include "IdParser.h"
#include "NumberParser.h"
#include "VariableValueParser.h"
#include "SymbolParser.h"
#include "FunctionParser.h"
#include "ReturnParser.h"
#include "BinOpParser.h"
#include "FCallParser.h"
#include "FCallStmtParser.h"
#include "FunctionSigParser.h"
#include "StringParser.h"
#include "CastExpr.h"
#include "AddressParser.h"
#include "PtrAssignParser.h"
#include "PtrValueParser.h"
#include "SizeofParser.h"
#include "ParenParser.h"
#include <iostream>

void Parser::init(CompileContext *cc){
  this->cc = cc;
  this->registerTopRule(new DefineParser());


  this->registerExpressionRule(new AddressParser());
  this->registerExpressionRule(new CastExpr());
  this->registerExpressionRule(new BinOpParser());
  this->registerExpressionRule(new NumberParser());
  this->registerExpressionRule(new FCallParser());
  this->registerExpressionRule(new VariableValueParser());
  this->registerExpressionRule(new StringParser());
  this->registerExpressionRule(new PtrValueParser());
  this->registerExpressionRule(new SizeofParser());
  this->registerExpressionRule(new ParenParser());

  this->registerValueRule(new FunctionParser());
  this->registerValueRule(new FunctionSigParser());

  this->registerStatementRule(new FCallStmtParser());
  this->registerStatementRule(new AssignParser());
  this->registerStatementRule(new PtrAssignParser());
  this->registerStatementRule(new DefineParser());
  this->registerStatementRule(new ReturnParser());
  this->registerStatementRule(new IfParser());
  this->registerStatementRule(new WhileParser());
  this->registerStatementRule(new SBlockParser());

  this->registerTypeRule(new PrimitiveTypeParser());
  this->registerTypeRule(new PointerTypeParser());
  this->registerTypeRule(new FunctionTypeParser());
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

int skipwscomment(std::string *w, int pos){
  pos = skipws(w, pos);
  if(pos == -1) return -1;
  char c = w->at(pos);
  int len = w->size();
  while(c == '/'){
    c = w->at(pos+1);
    if(c == '/'){
      // line comments
      while(w->at(pos) != '\n' && pos < len) pos++;
      if(pos == len) return -1;
      pos = skipws(w, pos);
      c = w->at(pos);
    }

    if(c == '*'){
      pos ++;
      // block comments
      int level = 1;
      while(level){
        pos ++;
        if(w->at(pos) == '/' && w->at(pos+1) == '*'){
          level ++;
          pos ++;
        }
        if(w->at(pos) == '*' && w->at(pos+1) == '/'){
          pos += 2;
          level --;
        }
        if(pos == len) return -1;
      }
      pos = skipws(w, pos);
      c = w->at(pos);
    }
  }
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

void Parser::registerExpressionRule(ParseRule *p){
  expressionRules.push_back(p);
}

void Parser::registerValueRule(ParseRule *p){
  valueRules.push_back(p);
}

void Parser::registerStatementRule(ParseRule *p){
  statementRules.push_back(p);
}

ParseResult Parser::parseTop(Source *s, int pos){
  // TODO
  // TODO produce an error when you can't parse
  ParseResult ans;
  for(auto r: topRules){
    ans = r->parse(cc, s, pos);
    if(ans) return ans;
  }
  return 0;
}

ParseResult Parser::parseType(Source *s, int pos){
  // TODO
  ParseResult ans;
  for(auto r: typeRules){
    ans = r->parse(cc, s, pos);
    if(ans) return ans;
  }
  return 0;
}

ParseResult Parser::parseExpression(Source *s, int pos){
  // TODO
  ParseResult ans;
  for(auto r: expressionRules){
    ans = r->parse(cc, s, pos);
    if(ans) return ans;
  }
  return 0;
}

ParseResult Parser::parseValue(Source *s, int pos){
  // TODO
  ParseResult ans;
  for(auto r: valueRules){
    ans = r->parse(cc, s, pos);
    if(ans) return ans;
  }
  return 0;
}

ParseResult Parser::parseStatement(Source *s, int pos){
  // TODO
  ParseResult ans;
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


ParseResult operator>>(ParseResult t, ParseRule &p2){
  if(!t) return t;
  ParseResult ans = p2.parse(t.token->cc, t.token->s, t.token->epos + 1);
  if(!ans) return ans;
  return ParseResult(new TupleToken(t.token, ans.token));
}


TupleToken::TupleToken(Token *t1, Token *t2):t1(t1),t2(t2){
  this->spos = t1->spos;
  this->epos = t2->epos;
  this->s = t1->s;
  this->cc = t1->cc;
}

TupleToken::~TupleToken(){
  // TODO
  //delete t1;
  //delete t2;
}

std::string TupleToken::desc(){
  return t1->desc() + " >> " + t2->desc();
}

ParseResult ParseRule::parse(CC *cc, Source *s, int pos){
  // TODO memoization
  pos = skipwscomment(&s->str, pos);
  if(pos == -1) return 0;
  return this->scheme(cc, s, pos);
}

int extractNextAlphaNumerical(std::string *str, int pos){
  int end = pos;
  int len = str->size();
  char c = str->at(pos);
  if(!isalpha(c)) return -1;
  do{
    end++;
    if(end >= len){
      break;
    }
    c = str->at(end);
  } while(isalphanumerical(c));
  return end;
}


ParseResult::~ParseResult(){
  if(token){
    // TODO
    // delete token;
  }
}


std::ostream &operator<<(std::ostream &os, ParseResult &s){
  return os << *s.token;
}

Token::~Token(){
}
