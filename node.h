#pragma once
#include "type.h"
#include <string>
#include <sstream>
#include <vector>

class Node{
public:
  int lineno;
  Node(){
    extern int yylineno;	// defined and maintained in lex.c
    lineno = yylineno;
  }

  virtual ~Node(){
  }
};

class Statement: public Node{
};

class Expression: public Statement{
public:
  Type *exprType = 0;
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
  bool dynamicStack = false; // Wether we have dynamic stack, i.e., stack array of dynamic size
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
  Expression *base;
  Expression *exp;
  VariableAssign(Expression *b, Expression *exp): base(b), exp(exp){
  }

  virtual ~VariableAssign(){
    delete base;
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

class DeferStatement: public Statement{
public:
  Statement *s;

  DeferStatement(Statement *s): s(s){
  }

  virtual ~DeferStatement(){
    delete s;
  }
};

class StructStatement: public Statement{
public:
  std::string *name;
  std::vector<VariableDecl *> *members;
  StructStatement(std::string *name, std::vector<Statement *> *m): name(name){
    members = new std::vector<VariableDecl *>();
    for(auto s: *m)
      members->push_back((VariableDecl *) s);
  }

  virtual ~StructStatement(){
    delete name;
    for(auto s: *members)
      delete s;
    delete members;
  }
};

class MemberExpr: public Expression{
public:
  Expression *e;
  std::string *mem;
  MemberExpr(Expression *e, std::string *mem): mem(mem), e(e){}

  virtual ~MemberExpr(){
    delete e;
    delete mem;
  }
};

class ArrayExpr: public Expression{
public:
  Expression *e;
  Expression *mem;
  ArrayExpr(Expression *e, Expression *mem): mem(mem), e(e){}

  virtual ~ArrayExpr(){
    delete e;
    delete mem;
  }
};

class CastExpr: public Expression{
public:
  Type *t;
  Expression *exp;
  CastExpr(Type *t, Expression *e): t(t), exp(e){
  }

  virtual ~CastExpr(){
    delete t;
    delete exp;
  }
};

class PointerExpr: public Expression{
public:
  Expression *exp;
  PointerExpr(Expression *e):exp(e){}

  virtual ~PointerExpr(){
    delete exp;
  }
};

class PointerAccessExpr: public Expression{
public:
  Expression *exp;
  PointerAccessExpr(Expression *e):exp(e){}

  virtual ~PointerAccessExpr(){
    delete exp;
  }
};

class ArrayType: public Type{
public:
  Type *base;
  int count;
  Expression *exp;

  ArrayType(Type *b, Expression *e): base(b), exp(e){}
  ArrayType(Type *b, int count): base(b), count(count){}

  virtual ArrayType *clone(){
    if(count){
      return new ArrayType(base->clone(), count);
    }
    return new ArrayType(base->clone(), exp);
  }

  virtual std::string displayName(){
    if(count)
      return "Array of " + base->displayName() + " of size " + std::to_string(count);
    return "Array of " + base->displayName();
  }

  virtual ~ArrayType(){
    delete base;
    if(exp)
      delete exp;
  }
};
