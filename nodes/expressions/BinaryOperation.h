#pragma once
#include "../Expression.h"

enum Operation{
    PLUS = 1,
    MULT = 2,
    SUB = 3,
    DIV = 4,
    MOD = 5,
    EQ = 6,
};

class BinaryOperation: public Expression{
public:
  Expression *lhs, *rhs;
  Operation op;
  BinaryOperation(Expression *l, Operation op, Expression *r): op(op), lhs(l), rhs(r){}

  virtual ~BinaryOperation();
  virtual llvm::Value *exprGen(CodegenContext *cc);
  virtual llvm::Value *getAlloca(CodegenContext *cc);
  virtual void compile(CompileContext *cc);
  virtual Type *resolveType(CompileContext *cc);
};
