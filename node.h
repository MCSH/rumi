#pragma once
#include "type.h"
#include <algorithm>
#include <functional>
#include <string>
#include <sstream>
#include <vector>
#include <map>

#include <llvm/IR/Value.h>

class CodegenContext;

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
public:
  virtual void codegen(CodegenContext *cc)=0;
};

class Expression: public Statement{
public:
  Type *exprType = 0;
  virtual llvm::Value *exprGen(CodegenContext *cc)=0;
  virtual llvm::Value *getAlloca(CodegenContext *cc)=0; // TODO maybe extend Expression for this because most don't have it
  virtual void codegen(CodegenContext *cc){} // Almost all of expressions have empty codegen
};

class IntValue: public Expression{
public:
  std::string *val=0;
  int size;

  IntValue(std::string *val): val(val){
  }

  IntValue(int size): size(size){}

  virtual ~IntValue(){
    if(val)
      delete val;
  }
  virtual llvm::Value *exprGen(CodegenContext *cc);
  virtual llvm::Value *getAlloca(CodegenContext *cc){}
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
  virtual llvm::Value *exprGen(CodegenContext *cc);
  virtual llvm::Value *getAlloca(CodegenContext *cc){}
};

class ReturnStatement: public Statement{
public:
  Expression *exp;
  ReturnStatement(Expression *e): exp(e){}

  ReturnStatement(){
    exp = 0;
  }

  virtual ~ReturnStatement(){
    delete exp;
  }
  virtual void codegen(CodegenContext *cc);
  virtual llvm::Value *getAlloca(CodegenContext *cc){}
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
  virtual void codegen(CodegenContext *cc);
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
  bool isLocal = false;
  FunctionSignature(std::string *val, std::vector<Statement *> *a, Type *t): name(val), returnT(t){
    args = new std::vector<ArgDecl*>();
    if(a){
      for (auto s : *a) {
        args->push_back((ArgDecl *)s);
      }
      delete a;
    }
  }

  Type *fType=0;

  Type *getType();

  virtual ~FunctionSignature(){
    delete name;
    delete returnT;

    for(auto s: *args){
      delete s;
    }
    delete args;
  }
  virtual void codegen(CodegenContext *cc);
  llvm::Function *signgen(CodegenContext *cc);
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
  virtual void codegen(CodegenContext *cc);
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
  llvm::Function *funcgen(CodegenContext *cc);
  virtual void codegen(CodegenContext *cc);
};

class VariableExpr: public Expression{
public:
  std::string *name;
  VariableExpr(std::string *n): name(n){
    
  }

  virtual ~VariableExpr(){
    delete name;
  }
  virtual llvm::Value *exprGen(CodegenContext *cc);
  virtual llvm::Value *getAlloca(CodegenContext *cc);
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
  virtual void codegen(CodegenContext *cc);
};


class FunctionCallExpr: public Expression{
public:
  std::string *name;
  std::vector<Expression *> *expr;

  FunctionCallExpr(std::string *n, std::vector<Expression *> *expr): name(n), expr(expr){
    if(!expr)
      this->expr = new std::vector<Expression*>();
  }
 
  virtual ~FunctionCallExpr(){
    delete name;
    for (auto e : *expr)
      delete e;
    delete expr;
  }
  virtual llvm::Value *exprGen(CodegenContext *cc);
  virtual llvm::Value *getAlloca(CodegenContext *cc){}
  void codegen(CodegenContext *cc);
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
  virtual llvm::Value *exprGen(CodegenContext *cc);
  virtual llvm::Value *getAlloca(CodegenContext *cc){}
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
  virtual void codegen(CodegenContext *cc);
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
  virtual void codegen(CodegenContext *cc);
};

class DeferStatement: public Statement{
public:
  Statement *s;

  DeferStatement(Statement *s): s(s){
  }

  virtual ~DeferStatement(){
    delete s;
  }
  virtual void codegen(CodegenContext *cc);
};

class StructStatement: public Statement{
public:
  std::string *name;
  std::vector<VariableDecl *> *members;
  std::map<std::string, FunctionDefine *> methods;
  bool has_initializer = false;
  int type_counter;
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
  virtual void codegen(CodegenContext *cc);
};

class MemberExpr: public Expression{
public:
  Expression *e;
  std::string *mem;
  int level=0;
  MemberExpr(Expression *e, std::string *mem): mem(mem), e(e){}

  virtual ~MemberExpr(){
    delete e;
    delete mem;
  }
  virtual llvm::Value *exprGen(CodegenContext *cc);
  virtual llvm::Value *getAlloca(CodegenContext *cc);
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
  virtual llvm::Value *exprGen(CodegenContext *cc);
  virtual llvm::Value *getAlloca(CodegenContext *cc);
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
  virtual llvm::Value *exprGen(CodegenContext *cc);
  virtual llvm::Value *getAlloca(CodegenContext *cc){}
};

class PointerExpr: public Expression{
public:
  Expression *exp;
  PointerExpr(Expression *e):exp(e){}

  virtual ~PointerExpr(){
    delete exp;
  }
  virtual llvm::Value *exprGen(CodegenContext *cc);
  virtual llvm::Value *getAlloca(CodegenContext *cc){}
};

class PointerAccessExpr: public Expression{
public:
  Expression *exp;
  PointerAccessExpr(Expression *e):exp(e){}

  virtual ~PointerAccessExpr(){
    delete exp;
  }
  virtual llvm::Value *exprGen(CodegenContext *cc);
  virtual llvm::Value *getAlloca(CodegenContext *cc);
};

class SizeofExpr: public Expression{
public:
  Type *t;

  SizeofExpr(Type *t): t(t){}

  virtual ~SizeofExpr(){
    delete t;
  }
  virtual llvm::Value *exprGen(CodegenContext *cc);
  virtual llvm::Value *getAlloca(CodegenContext *cc){}
};

class TypeNode: public Expression{
public:
  Type *exprType;
  TypeNode(Type *exprType): exprType(exprType){}
  virtual llvm::Value *exprGen(CodegenContext *cc){}
  virtual llvm::Value *getAlloca(CodegenContext *cc){}
};


class ImportStatement: public Statement{
public:
  std::string *name;
  std::vector<Statement *> *stmts;
  ImportStatement(std::string *n): name(n){}
  virtual ~ImportStatement(){
    delete name;
    if(stmts)
      delete stmts;
  }
  virtual void codegen(CodegenContext *cc);
};


class CompileStatement: public Statement{
public:
  std::string *name;
  Statement *s;
  CompileStatement(std::string *n, Statement *s): name(n), s(s){}
  virtual ~CompileStatement(){
    delete name;
    delete s;
  }
  virtual void codegen(CodegenContext *cc);
};

class MemberStatement: public Statement{
public:
  std::string *name;
  FunctionDefine *f;
  MemberStatement(std::string *n, Statement *f): name(n), f((FunctionDefine*)f){
    // Change the function name to prevent name colision
  }

  virtual ~MemberStatement(){
    delete name;
    delete f;
  }

  virtual void codegen(CodegenContext *cc);
};

class MethodCall: public Expression{
public:
  Expression *e;
  std::string *name;
  std::vector<Expression *> *expr;

  FunctionCallExpr *fce;
  FunctionDefine *f;

  int methodInd; // used for interfaces

  MethodCall(Expression *e, std::string *n, std::vector<Expression *> *expr): e(e), name(n), expr(expr){
    if(!expr){
      this->expr = new std::vector<Expression*>();
    }
  }
 
  virtual ~MethodCall(){
    delete e;
    delete name;
    for (auto e : *expr)
      delete e;
    delete expr;
  }
  virtual llvm::Value *exprGen(CodegenContext *cc);
  virtual llvm::Value *getAlloca(CodegenContext *cc){}
  void codegen(CodegenContext *cc);
};


class InterfaceStatement: public Statement{
public:
  std::string *name;
  std::vector<FunctionSignature *> *members;

  std::map<std::string, StructStatement*> implements;

  InterfaceStatement(std::string *n, std::vector<Statement *> *m): name(n){
    members = new std::vector<FunctionSignature*>();
    for(auto i: *m){
      members->push_back((FunctionSignature *)i);
    }
    delete m;
  }

  virtual ~InterfaceStatement(){
    delete name;
    for(auto i: *members)
      delete i;
    delete members;
  }
  virtual void codegen(CodegenContext *cc);
};
