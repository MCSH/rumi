#pragma once
#include "type.h"
#include <algorithm>
#include <string>
#include <sstream>
#include <vector>
#include <map>

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
  int size;

  IntValue(std::string *val): val(val){
  }

  IntValue(int size): size(size){}

  virtual ~IntValue(){
    if(val)
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
  ReturnStatement(Expression *e): exp(e){}

  ReturnStatement(){
    exp = 0;
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

  Type *fType;

  Type *getType();

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
      this->expr = new std::vector<Expression*>();
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
  std::map<std::string, FunctionDefine *> methods;
  bool has_initializer = false;
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
  Type *base = 0;
  int count = 0;
  Expression *exp = 0;

  ArrayType(Type *b, Expression *e): base(b), exp(e){}
  ArrayType(Type *b, int count): base(b), count(count){}
  ArrayType(Type *b): base(b){}

  virtual ArrayType *clone(){
    if(count){
      return new ArrayType(base->clone(), count);
    }
    if(exp)
      return new ArrayType(base->clone(), exp);
    return new ArrayType(base->clone());
  }

  virtual Compatibility compatible(Type *t){
    auto tid = typeid(*t).hash_code();
    if(tid!= typeid(ArrayType).hash_code() && tid!=typeid(PointerType).hash_code())
      return Compatibility::UNCOMPATIBLE;

    if(tid == typeid(PointerType).hash_code())
      return base->compatible(((PointerType*)t)->base);

    auto at = (ArrayType*) t;

    // TODO check for size, etc

    return base->compatible(at->base);
  }

  virtual std::string displayName(){
    if(count)
      return "Array of " + base->displayName() + " of size " + std::to_string(count);
    if(exp)
      return "Array of " + base->displayName() + " of unknown size at runtime";
    return "Array of " + base->displayName();
  }

  virtual ~ArrayType(){
    delete base;
    if(exp)
      delete exp;
  }
};

class SizeofExpr: public Expression{
public:
  Type *t;

  SizeofExpr(Type *t): t(t){}

  virtual ~SizeofExpr(){
    delete t;
  }
};

class TypeNode: public Expression{
public:
  Type *exprType;
  TypeNode(Type *exprType): exprType(exprType){}
};

class FunctionType: public Type{
public:
  Type *returnType;
  std::vector<Type*> *args;
  FunctionType(std::vector<Statement *> *args, Type* returnType): returnType(returnType){
    this->args = new std::vector<Type*>();
    for(auto a: *args){
      TypeNode *tn = (TypeNode *) a;
      this->args->push_back(tn->exprType->clone());
      delete tn;
    }
    delete args;
  }

  FunctionType(std::vector<Type *> *args, Type *returnType): returnType(returnType), args(args){}

  virtual FunctionType *clone(){
    // TODO
    std::vector<Type *> *nargs = new std::vector<Type *>();

    for(auto a: *args){
      nargs->push_back(a->clone());
    }

    return new FunctionType(nargs, returnType->clone());
  }


  virtual std::string displayName(){

    std::string argsName = "";
    for(auto arg: *args){
      argsName = argsName + arg->displayName() + ", ";
    }

    return "Function of type ("+argsName+")->"+returnType->displayName();
  }

  virtual Compatibility compatible(Type *t){
    if(typeid(*t).hash_code()!= typeid(FunctionType).hash_code())
      return Compatibility::UNCOMPATIBLE;

    // TODO this could be improved

    auto ft = (FunctionType*) t;

    if(ft->args->size() != args->size())
      return Compatibility::UNCOMPATIBLE;

    Compatibility ans = returnType->compatible(ft->returnType);

    for(int i = 0 ; i < args->size(); i ++){
      Type *tmp = (*args)[i];
      Type *tmp2 = (*ft->args)[i];
      ans = std::max(ans, tmp->compatible(tmp2));
    }
    
    return ans;
  }

  virtual ~FunctionType(){
    delete returnType;
    delete args;
  }
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
};

class MethodCall: public Expression{
public:
  Expression *e;
  std::string *name;
  std::vector<Expression *> *expr;

  FunctionCallExpr *fce;
  FunctionDefine *f;

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
};
