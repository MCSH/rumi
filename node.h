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
public:
  std::string *val;
  IntValue(std::string *val): val(val){
    // TODO 
  }

  virtual ~IntValue(){
    delete val;
  }
};

class ReturnStatement: public Statement{
public:
  Expression *exp;
  ReturnStatement(Expression *e): exp(e){
    // TODO
  }

  virtual ~ReturnStatement(){
    delete exp;
  }
};

class FunctionSignature: public Statement{
public:
  std::string *name;
  Type *returnT;
  FunctionSignature(std::string *val, Type *t): name(val), returnT(t){
    // TODO
  }

  virtual ~FunctionSignature(){
    delete name;
    delete returnT;
  }
};

class FunctionBody: public Statement{
public:
  std::vector<Statement *> *stmts;
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
public:
  FunctionSignature *sign;
  FunctionBody *body;
  FunctionDefine(FunctionSignature *d, FunctionBody *b): sign(d), body(b){
    // TODO
  }

  virtual ~FunctionDefine(){
    delete sign;
    delete body;
  }
};
