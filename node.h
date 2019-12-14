#pragma once
#include "type.h"
#include <string>
#include <sstream>
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
  StringValue(std::string *v){
    std::stringstream ss{""};

    for (int i = 1; i < v->length() - 1; i++) {
      if (v->at(i) == '\\') {
        switch (v->at(i + 1)) {
        case 'n':
          ss << '\n';
          i++;
          break;
        case '"':
          ss << '\"';
          i++;
          break;
        default:
          ss << '\\';
        }
      } else {
        ss << v->at(i);
      }
    }

    this->val = new std::string(ss.str());
    delete v;
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

class CodeBlock: public Statement{
public:
  std::vector<Statement *> *stmts;
  CodeBlock(std::vector<Statement*> *s): stmts(s){
  }

  virtual ~CodeBlock(){
    for(auto s: *stmts){
      delete s;
    }

    delete stmts;
  }
};

class FunctionDefine: public Statement{
public:
  FunctionSignature *sign;
  CodeBlock *body;
  FunctionDefine(FunctionSignature *d, CodeBlock *b): sign(d), body(b){
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

class IfStatement: public Statement{
public:
  Statement *i, *e;
  Expression *exp;

  IfStatement(Expression *exp, Statement *i, Statement *e=nullptr): i(i), e(e), exp(exp){
  }

  virtual ~IfStatement(){
    delete i;
    if(e) delete e;
    delete exp;
  }
};


class WhileStatement: public Statement{
public:
  Statement *w;
  Expression *exp;

  WhileStatement(Expression *exp, Statement *w): w(w), exp(exp){
  }

  virtual ~WhileStatement(){
    delete w;
    delete exp;
  }
};
