#pragma once
#include <llvm/IR/Instructions.h>
#include <vector>
#include <string>
#include <map>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"

class CC;

class Type {
public:
  int lineno;
  Type() {
    extern int yylineno;
    lineno = yylineno;
  }
  virtual llvm::Type *typegen(CC *cc) = 0;
};

class Expression {
public:
  int lineno;
  Expression(){
    extern int yylineno;
    lineno = yylineno;
  }
  virtual llvm::Value *exprgen(CC *cc)=0;
  virtual llvm::Value *getAlloca(CC *cc)=0;
  virtual Type *gettype(CC *cc)=0;
};

class Statement {
public:
  virtual void compile(CC *cc)=0;
};

class VariableDecl : public Statement {
public:
  std::string *name;
  Type *type;
  Expression *expr;
  VariableDecl(){} // Just for ArgDecl
  VariableDecl(std::string *name, Type *type=0, Expression *expr=0):name(name), type(type), expr(expr){}
  virtual void compile(CC *cc);
};


class ArgDecl : public VariableDecl {
public:
  bool is_vararg;
  ArgDecl(std::string *name, Type *type, bool is_vararg=false): is_vararg(is_vararg){
    this->name = name;
    this->type = type;
  }
  void compile(CC *cc){}
};


class FunctionSignature : public Statement {
public:
  std::string *name;
  std::vector<ArgDecl *> *args;
  Type *type;

  FunctionSignature(std::string *name, std::vector<Statement *> *args, Type *type): name(name), type(type){
    this->args = new std::vector<ArgDecl *>();
    if(args)
    for(auto a: *args){
      this->args->push_back((ArgDecl *) a);
    }
    
  }
  void compile(CC *cc);
  llvm::Function *signgen(CC *cc);

};
class CodeBlock : public Statement {
public:
  std::vector<Statement *> *stmts;
  CodeBlock(std::vector<Statement *> *stmts): stmts(stmts){}
  void compile(CC *cc);
};

class FunctionDefine : public Statement {
public:
  FunctionSignature *fs;
  CodeBlock *cb;

  FunctionDefine(FunctionSignature *fs, CodeBlock *cb): fs(fs), cb(cb){}
  void compile(CC *cc);
};

class StructStatement : public Statement {
public:
  std::string *name;
  std::vector<VariableDecl *> *args;
  StructStatement(std::string *name, std::vector<Statement *> *args): name(name){
    this->args = new std::vector<VariableDecl *>();
    for(auto a: *args){
      this->args->push_back((VariableDecl *) a);
    }
  }
  void compile(CC *cc);
};

class MemberExpr : public Expression {
public:
  std::string *name;
  Expression *base;
  MemberExpr(Expression *base, std::string *name): base(base), name(name){}
  llvm::Value *exprgen(CC *cc);
  llvm::Value *getAlloca(CC *cc);
  Type *gettype(CC *cc);
};

class WhileStatement : public Statement {
public:
  Expression *expr;
  Statement *stmt;
  WhileStatement(Expression *expr, Statement *stmt): expr(expr), stmt(stmt){}
  void compile(CC *cc);
};
class IfStatement : public Statement {
public:
  Expression *expr;
  Statement *stmt1, *stmt2;
  IfStatement(Expression *expr, Statement *stmt1, Statement *stmt2=0): expr(expr), stmt1(stmt1), stmt2(stmt2){}
  void compile(CC *cc);
};

class VariableAssign : public Statement {
public:
  Expression *var, *expr;
  VariableAssign(Expression *var, Expression *expr): var(var), expr(expr){}
  void compile(CC *cc);
};

class ReturnStatement : public Statement {
public:
  Expression *expr;
  ReturnStatement(Expression *expr=0):expr(expr){}
  void compile(CC *cc);
};

class VariableExpr : public Expression {
public:
  std::string *name;
  VariableExpr(std::string *name): name(name){}
  llvm::Value *exprgen(CC *cc);
  llvm::Value *getAlloca(CC *cc);
  Type *gettype(CC *cc);
};

enum Operation{
               EQ, PLUS, SUB, MULT, DIV, MOD
};

class BinaryOperation : public Expression {
public:
  Expression *lhs, *rhs;
  Operation ops;
  BinaryOperation(Expression *lhs, Operation ops, Expression *rhs): lhs(lhs), ops(ops), rhs(rhs){}
  llvm::Value *exprgen(CC *cc);
  llvm::Value *getAlloca(CC *cc){}
  Type *gettype(CC *cc){return lhs->gettype(cc);}
};

class PointerExpr : public Expression {
public:
  Expression *ptr;
  PointerExpr(Expression *ptr): ptr(ptr){}
  llvm::Value *exprgen(CC *cc);
  llvm::Value *getAlloca(CC *cc);
  Type *gettype(CC *cc);
};

class FunctionCallExpr : public Expression {
public:
  std::string *name;
  std::vector<Expression *> *args;
  FunctionCallExpr(std::string *name, std::vector<Expression*> *args): name(name), args(args){}
  llvm::Value *exprgen(CC *cc);
  llvm::Value *getAlloca(CC *cc){};
  Type *gettype(CC *cc);
};

class IntType : public Type {
public:
  int size;
  bool sign;
  IntType(int size=0, int sign=true): sign(sign), size(size){}
  llvm::Type *typegen(CC *cc);
};

class IntValue : public Expression {
public:
  long long int val;
  IntValue(std::string *val): val(atoi(val->c_str())){}
  llvm::Value *exprgen(CC *cc);
  llvm::Value *getAlloca(CC *cc){}
  Type *gettype(CC *cc){return new IntType();}
};

class FloatType : public Type {
public:
  int size;
  FloatType(int size): size(size){}
  llvm::Type *typegen(CC *cc);
};

class StringType : public Type {
public:
  llvm::Type *typegen(CC *cc);
};

class PointerAccessExpr : public Expression {
public:
  Expression *ptr;
  PointerAccessExpr(Expression *ptr): ptr(ptr){}
  llvm::Value *exprgen(CC *cc);
  llvm::Value *getAlloca(CC *cc){
    return ptr->exprgen(cc);
  }
  Type *gettype(CC *cc);
};

class PointerType : public Type {
public:
  Type *base;
  PointerType(Type *base): base(base){}
  llvm::Type *typegen(CC *cc);
};

class StringValue: public Expression{
public:
  std::string *val;
  StringValue(std::string *val);
  llvm::Value *exprgen(CC *cc);
  llvm::Value *getAlloca(CC *cc){
    printf("Can't assignt to string on line %d\n", lineno);
    exit(1);
  }
  Type *gettype(CC *cc){return new StringType();}
};


class VoidType : public Type {
public:
  llvm::Type *typegen(CC *cc);
};

class AnyType : public Type {
public:
  llvm::Type *typegen(CC *cc);
};

class StructType: public Type{
public:
  std::string *name;
  StructType(std::string *name):name(name){}
  llvm::Type *typegen(CC *cc);
};

class BC{
public:
  std::map<std::string, VariableDecl *> vars;
  std::map<std::string, llvm::AllocaInst *> alloca;

  std::map<std::string, StructStatement *> structs;
  std::map<std::string, llvm::StructType *> stypes;

  std::map<std::string, FunctionSignature *> funcs;
  
  llvm::BasicBlock *bblock;
  BC(llvm::BasicBlock *bblock=0):bblock(bblock){}
};

class CC {
public:

  BC *global;
  std::vector<BC *> blocks;

  CC(){
    global = new BC();
  }

  llvm::LLVMContext context;
  llvm::Module *module;
  llvm::IRBuilder<> *builder;

  void setVariable(std::string *name, llvm::AllocaInst *alloca, VariableDecl * vd);
  void setStruct(std::string *name, llvm::StructType *st, StructStatement *s);

  llvm::AllocaInst *getVariable(std::string *name);
  llvm::StructType *getStructType(std::string *name);
  VariableDecl *getVariableDecl(std::string *name);
  StructStatement *getStruct(std::string *name);
  FunctionSignature *getFunction(std::string *name);
};
