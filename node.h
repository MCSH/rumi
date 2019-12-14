#pragma once
#include "type.h"
#include <string>
#include <vector>

class Node{
public:
  virtual ~Node(){
  }
};

class Statement: public Node{
};

class Expression: public Statement{
public:
};

class IntValue: public Expression{
public:
  std::string *val;
  IntValue(std::string *val): val(val){
  }

  virtual ~IntValue(){
    delete val;
  }
};

class StringValue: public Expression{
public:
  std::string *val;
  StringValue(std::string *val): val(val){
  }

  virtual ~StringValue(){
    delete val;
  }
};

class ReturnStatement: public Statement{
public:
  Expression *exp;
  ReturnStatement(Expression *e): exp(e){
  }

  virtual ~ReturnStatement(){
    delete exp;
  }
};


class VariableDecl: public Statement{
public:
  std::string *name;
  Type *t;
  Expression *exp;
  VariableDecl(){}
  VariableDecl(std::string *n, Type *t, Expression *e=nullptr): name(n), t(t), exp(e){}

  virtual ~VariableDecl(){
    delete name;
    delete t;
    delete exp;
  }
};


class ArgDecl: public VariableDecl{
public:
  std::string *name;
  Type *t;
  bool vardiac;
  ArgDecl(std::string *n, Type *t, bool v=false): name(n), t(t), vardiac(v){
  }

  virtual ~ArgDecl(){
    delete name;
    delete t;
  }
};

class FunctionSignature: public Statement{
public:
  std::string *name;
  Type *returnT;
  std::vector<ArgDecl *> *args;
  FunctionSignature(std::string *val, std::vector<Statement *> *a, Type *t): name(val), returnT(t){
    args = new std::vector<ArgDecl*>();
    if(a){
      for (auto s : *a) {
        args->push_back((ArgDecl *)s);
      }
      delete a;
    }
  }

  virtual ~FunctionSignature(){
    delete name;
    delete returnT;

    for(auto s: *args){
      delete s;
    }
    delete args;
  }
};

class FunctionBody: public Statement{
public:
  std::vector<Statement *> *stmts;
  FunctionBody(std::vector<Statement*> *s): stmts(s){
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
  }

  virtual ~FunctionDefine(){
    delete sign;
    delete body;
  }
};

class VariableExpr: public Expression{
public:
  std::string *name;
  VariableExpr(std::string *n): name(n){
    
  }

  virtual ~VariableExpr(){
    delete name;
  }
};

class VariableAssign: public Statement{
public:
  std::string *name;
  Expression *exp;
  VariableAssign(std::string *n, Expression *exp): name(n), exp(exp){
  }

  virtual ~VariableAssign(){
    delete name;
    delete exp;
  }
};


class FunctionCallExpr: public Expression{
public:
  std::string *name;
  std::vector<Expression *> *expr;

  FunctionCallExpr(std::string *n, std::vector<Expression *> *expr): name(n), expr(expr){
    if(!expr)
      expr = new std::vector<Expression*>();
  }
 
  virtual ~FunctionCallExpr(){
    delete name;
    for (auto e : *expr)
      delete e;
    delete expr;
  }
};

enum Operation{
    PLUS = 1,
    MULT = 2,
    SUB = 3,
    DIV = 4,
    MOD = 5,
};

class BinaryOperation: public Expression{
public:
  Expression *lhs, *rhs;
  Operation op;
  BinaryOperation(Expression *l, Operation op, Expression *r): op(op), lhs(l), rhs(r){}

  virtual ~BinaryOperation(){
    delete lhs;
    delete rhs;
  }
};
