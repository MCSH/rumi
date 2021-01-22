#include "parser.h"
#include "../Source.h"

#include "AssignParser.h"
#include "DefineParser.h"
#include "ExpressionParser.h"
#include "FunctionTypeParser.h"
#include "IfParser.h"
#include "PointerTypeParser.h"
#include "PrimitiveTypeParser.h"
#include "SBlockParser.h"
#include "StatementParser.h"
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
#include "PtrValueParser.h"
#include "SizeofParser.h"
#include "ParenParser.h"
#include "StructParser.h"
#include "InterfaceParser.h"
#include "NamedTypeParser.h"
#include "MemAccessParser.h"
#include "MethodParser.h"
#include "MethodCallParser.h"
#include "MethodCallStmtParser.h"
#include "BoolValueParser.h"
#include "PreOpParser.h"
#include "ImportParser.h"
#include "DirectiveParser.h"
#include "TopParser.h"
#include <iostream>

void Parser::init(CompileContext *cc){
  this->cc = cc;
  registerParser("define", new DefineParser());
  this->registerTopRule(new DefineParser());
  registerParser("struct", new StructParser());
  this->registerTopRule(new StructParser());
  registerParser("interface", new InterfaceParser());
  this->registerTopRule(new InterfaceParser());
  registerParser("method", new MethodParser());
  this->registerTopRule(new MethodParser());
  registerParser("import", new ImportParser());
  this->registerTopRule(new ImportParser());
  registerParser("directive", new DirectiveParser());
  this->registerTopRule(new DirectiveParser());

  registerParser("methodcall", new MethodCallParser());
  this->registerExpressionRule(new MethodCallParser());
  registerParser("memaccess", new MemAccessParser());
  this->registerExpressionRule(new MemAccessParser());
  registerParser("address", new AddressParser());
  this->registerExpressionRule(new AddressParser());
  registerParser("cast", new CastExpr());
  this->registerExpressionRule(new CastExpr());
  registerParser("binop", new BinOpParser());
  this->registerExpressionRule(new BinOpParser());
  registerParser("preop", new PreOpParser());
  this->registerExpressionRule(new PreOpParser());
  registerParser("number", new NumberParser());
  this->registerExpressionRule(new NumberParser());
  registerParser("fcall", new FCallParser());
  this->registerExpressionRule(new FCallParser());
  registerParser("variable", new VariableValueParser());
  this->registerExpressionRule(new VariableValueParser());
  registerParser("string", new StringParser());
  this->registerExpressionRule(new StringParser());
  registerParser("ptrval", new PtrValueParser());
  this->registerExpressionRule(new PtrValueParser());
  registerParser("sizeof", new SizeofParser());
  this->registerExpressionRule(new SizeofParser());
  registerParser("paren", new ParenParser());
  this->registerExpressionRule(new ParenParser());
  registerParser("bool", new BoolValueParser());
  this->registerExpressionRule(new BoolValueParser());

  registerParser("function", new FunctionParser());
  this->registerValueRule(new FunctionParser());
  registerParser("functionSignature", new FunctionSigParser());
  this->registerValueRule(new FunctionSigParser());

  registerParser("fcallstmt", new FCallStmtParser());
  this->registerStatementRule(new FCallStmtParser());
  registerParser("assign", new AssignParser());
  this->registerStatementRule(new AssignParser());
  // Already done
  this->registerStatementRule(new DefineParser());
  registerParser("return", new ReturnParser());
  this->registerStatementRule(new ReturnParser());
  registerParser("if", new IfParser());
  this->registerStatementRule(new IfParser());
  registerParser("while", new WhileParser());
  this->registerStatementRule(new WhileParser());
  registerParser("block", new SBlockParser());
  this->registerStatementRule(new SBlockParser());
  registerParser("methodcallstmt", new MethodCallStmtParser());
  this->registerStatementRule(new MethodCallStmtParser());

  registerParser("primtiveType", new PrimitiveTypeParser());
  this->registerTypeRule(new PrimitiveTypeParser());
  registerParser("pointerType", new PointerTypeParser());
  this->registerTypeRule(new PointerTypeParser());
  registerParser("functionType", new FunctionTypeParser());
  this->registerTypeRule(new FunctionTypeParser());
  registerParser("namedType", new NamedTypeParser());
  this->registerTypeRule(new NamedTypeParser());

  // main
  registerParser("expression", new ExpressionParser());
  registerParser("value", new ValueParser());
  registerParser("top", new TopParser());
  registerParser("statement", new StatementParser());
  registerParser("type", new TypeParser());
  registerParser("id", new IdParser());
  // TODO others? keywrod, symbol
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
        if(pos < len -2 && w->at(pos) == '/' && w->at(pos+1) == '*'){
          level ++;
          pos ++;
        }
        if(pos < len -2 && w->at(pos) == '*' && w->at(pos+1) == '/'){
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

ParseResult Parser::parseExpression(Source *s, int pos, int prec){
  // TODO
  ParseResult ans;
  for(auto r: expressionRules){
    if(prec > -1 && r->prec() > prec) continue;
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

int ParseRule::prec(){
  return 10;
}

ParseRule *Parser::getParserWithKey(std::string key){
  auto f = rules.find(key);
  if(f == rules.end()) return 0;
  return f->second;
}

void Parser::registerParser(std::string key, ParseRule *pr){
  rules[key] = pr;
}
