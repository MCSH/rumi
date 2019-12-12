#pragma once
#include "type.h"
#include <string>
#include <vector>

class Node{
public:
  Type *t;

  virtual ~Node(){
    delete t;
  }
};

class Expression: public Node{
};

class Statement: public Node{
};

class IntValue: public Expression{
  std::string *val;
public:
  IntValue(std::string *val): val(val){
    // TODO 
  }

  virtual ~IntValue(){
    delete val;
  }
};

class ReturnStatement: public Statement{
  Expression *exp;
public:
  ReturnStatement(Expression *e): exp(e){
    // TODO
  }

  virtual ~ReturnStatement(){
    delete exp;
  }
};

class FunctionDecl: public Statement{
  std::string *name;
  Type *returnT;
public:
  FunctionDecl(std::string *val, Type *t): name(val), returnT(t){
    // TODO
  }

  virtual ~FunctionDecl(){
    delete name;
    delete returnT;
  }
};

class FunctionBody: public Statement{
  std::vector<Statement *> *stmts;
public:
  FunctionBody(std::vector<Statement*> *s): stmts(s){
    // TODO
  }

  virtual ~FunctionBody(){
    for(auto s: *stmts){
      delete s;
    }

    delete stmts;
  }
};

class FunctionDefine: public Statement{
  FunctionDecl *decl;
  FunctionBody *body;
public:
  FunctionDefine(FunctionDecl *d, FunctionBody *b): decl(d), body(b){
    // TODO
  }

  virtual ~FunctionDefine(){
    delete decl;
    delete body;
  }
};
