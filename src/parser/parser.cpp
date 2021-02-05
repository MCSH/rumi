#include "parser.h"
#include "../Source.h"
#include "../DebugInfo.h"

#include "AssignParser.h"
#include "DefineParser.h"
#include "ExpressionParser.h"
#include "FunctionTypeParser.h"
#include "IfParser.h"
#include "PointerTypeParser.h"
#include "PrimitiveTypeParser.h"
#include "SBlockParser.h"
#include "StatementParser.h"
#include "Symbols.h"
#include "WhileParser.h"
#include "KeywordParser.h"
#include "IdParser.h"
#include "NumberParser.h"
#include "VariableValueParser.h"
#include "SymbolParser.h"
#include "FunctionParser.h"
#include "ReturnParser.h"
#include "BinOpParser.h"
#include "IndexingParser.h"
#include "FCallParser.h"
#include "FCallStmtParser.h"
#include "FunctionSigParser.h"
#include "StringParser.h"
#include "CastExprParser.h"
#include "AddressParser.h"
#include "PtrValueParser.h"
#include "SizeofParser.h"
#include "ParenParser.h"
#include "StructParser.h"
#include "EnumParser.h"
#include "InterfaceParser.h"
#include "NamedTypeParser.h"
#include "MemAccessParser.h"
#include "MethodParser.h"
#include "NewParser.h"
#include "BinOpDefParser.h"
#include "MethodCallParser.h"
#include "MethodCallStmtParser.h"
#include "BoolValueParser.h"
#include "PreOpParser.h"
#include "ImportParser.h"
#include "DirectiveParser.h"
#include "TopParser.h"
#include "UsingParser.h"
#include <iostream>
#include "Symbols.h"

class EmptyToken: public Token{
public:
  EmptyToken(CC *cc, Source *s, int spos, int epos){
    this->cc = cc;
    this->s = s;
    this->spos = spos;
    this->epos = epos;
  }
  virtual std::string desc() override { return ";"; }
  virtual AST *toAST(CC *cc) override { return 0; }
};

class EmptySemiColonParser: public ParseRule{
  SymbolParser sp;
public:
  EmptySemiColonParser():sp(s_semicolon){}
  ParseResult scheme(CC *cc, Source *s, int pos){
    auto a = sp.parse(cc, s, pos);
    if(!a) return a;
    return new EmptyToken(cc, s, a.token->spos, a.token->epos);
  }
};

void Parser::init(CompileContext *cc){
  this->cc = cc;
  this->registerTopRule(new DefineParser());
  this->registerTopRule(new StructParser());
  this->registerTopRule(new InterfaceParser());
  this->registerTopRule(new EnumParser());
  this->registerTopRule(new MethodParser());
  this->registerTopRule(new BinOpDefParser());
  this->registerTopRule(new NewParser());
  this->registerTopRule(new ImportParser());
  this->registerTopRule(new DirectiveParser());
  this->registerTopRule(new EmptySemiColonParser());

  this->registerExpressionRule(new BinOpParser());
  this->registerExpressionRule(new IndexingParser());
  this->registerExpressionRule(new MethodCallParser());
  this->registerExpressionRule(new MemAccessParser());
  this->registerExpressionRule(new AddressParser());
  this->registerExpressionRule(new CastExprParser());
  this->registerExpressionRule(new PreOpParser());
  this->registerExpressionRule(new NumberParser());
  this->registerExpressionRule(new FCallParser());
  this->registerExpressionRule(new VariableValueParser());
  this->registerExpressionRule(new StringParser());
  this->registerExpressionRule(new PtrValueParser());
  this->registerExpressionRule(new SizeofParser());
  this->registerExpressionRule(new ParenParser());
  this->registerExpressionRule(new BoolValueParser());

  this->registerValueRule(new FunctionParser());
  this->registerValueRule(new FunctionSigParser());

  this->registerStatementRule(new FCallStmtParser());
  this->registerStatementRule(new AssignParser());
  this->registerStatementRule(new DefineParser());
  this->registerStatementRule(new ReturnParser());
  this->registerStatementRule(new IfParser());
  this->registerStatementRule(new WhileParser());
  this->registerStatementRule(new SBlockParser());
  this->registerStatementRule(new MethodCallStmtParser());
  this->registerStatementRule(new EmptySemiColonParser());
  this->registerStatementRule(new UsingParser());

  this->registerTypeRule(new PrimitiveTypeParser());
  this->registerTypeRule(new PointerTypeParser());
  this->registerTypeRule(new FunctionTypeParser());
  this->registerTypeRule(new NamedTypeParser());
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
      while(pos < len && w->at(pos) != '\n') pos++;
      if(pos == len) return -1;
      pos = skipws(w, pos);
      if(pos == -1) return -1;
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
      if(pos == -1) return -1;
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

void Parser::registerTopRule(ParseRule *p, bool top){
  if(top)topRules.insert(topRules.begin(), p);
  else topRules.push_back(p);
}

void Parser::registerTypeRule(ParseRule *p, bool top){
  if(top) typeRules.insert(typeRules.begin(), p);
  else typeRules.push_back(p);
}

void Parser::registerExpressionRule(ParseRule *p, bool top){
  if(top) expressionRules.insert(expressionRules.begin(), p);
  else expressionRules.push_back(p);
}

void Parser::registerValueRule(ParseRule *p, bool top){
  if(top) valueRules.insert(valueRules.begin(), p);
  else valueRules.push_back(p);
}

void Parser::registerStatementRule(ParseRule *p, bool top){
  if(top) statementRules.insert(statementRules.begin(), p);
  else statementRules.push_back(p);
}

ParseResult Parser::parseTop(Source *s, int pos){
  // TODO
  // TODO produce an error when you can't parse
  ParseResult ans;
  for(auto r: topRules){
    ans = r->parse(cc, s, pos);
    if(ans) return ans;
  }
  return ParseResult();
}

ParseResult Parser::parseType(Source *s, int pos){
  // TODO
  ParseResult ans;
  for(auto r: typeRules){
    ans = r->parse(cc, s, pos);
    if(ans) return ans;
  }
  return ParseResult();
}

ParseResult Parser::parseExpression(Source *s, int pos, int prec){
  // TODO
  ParseResult ans;
  for(auto r: expressionRules){
    if(prec > -1 && r->prec() > prec) continue;
    ans = r->parse(cc, s, pos);
    if(ans) return ans;
  }
  return ParseResult();
}

ParseResult Parser::parseValue(Source *s, int pos){
  // TODO
  ParseResult ans;
  for(auto r: valueRules){
    ans = r->parse(cc, s, pos);
    if(ans) return ans;
  }
  return ParseResult();
}

ParseResult Parser::parseStatement(Source *s, int pos){
  // TODO
  ParseResult ans;
  for(auto r: statementRules){
    ans = r->parse(cc, s, pos);
    if(ans) return ans;
  }
  return ParseResult();
}

std::ostream &operator<<(std::ostream &os, Token &s) {
  os << *s.getDBG() << " : " << s.desc();
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
  if(pos == -1) return ParseResult();
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
  if(s.token)
    return os << *s.token;
  else if(s.failure)
    return os << "Failure"; // TODO
  else
    return os << "UNKNOWN PARSERESULT";
}

Token::~Token(){
}

int ParseRule::prec(){
  return 10;
}

DebugInfo *Token::getDBG(){
  return new DebugInfo(this);
}

AST *Token::getAST(CC *cc){
  auto ans = this->toAST(cc);
  if(ans) ans->setDBG(getDBG());
  return ans;
}
