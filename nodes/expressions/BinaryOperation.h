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
  virtual llvm::Value *exprGen(Context *cc);
  virtual llvm::Value *getAlloca(Context *cc);
  virtual void compile(Context *cc);
  virtual Type *resolveType(Context *cc);
};
