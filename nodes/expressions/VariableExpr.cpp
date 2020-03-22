#include "VariableExpr.h"
#include "../../Context.h"

VariableExpr::~VariableExpr() { delete name; }
llvm::Value *VariableExpr::exprGen(Context *cc) {
  llvm::AllocaInst *alloc = cc->getVariableAlloca(this->name);
  if (!alloc) {
    // is it a function?

    llvm::Function *f = cc->module->getFunction(*this->name);
    if (f) {
      // We have a function
      return f;
    }

    printf("Unknown variable %s\n", this->name->c_str());
    printf("This is a compiler bug reported in codegen::variableExprGen\n");
    exit(1);
  }

  auto load = cc->builder->CreateLoad(alloc, "readtmp");

  return load;
}

llvm::Value *VariableExpr::getAlloca(Context *cc) {
  return cc->getVariableAlloca(name);
}

void VariableExpr::compile(Context *cc) {
  resolveType(cc);
  if (!cc->getVariableType(name)) {
    printf("Unknown variable %s in line %d\n", name->c_str(), lineno);
    exit(1);
  }
}

Type *VariableExpr::resolveType(Context *cc) {
  if (exprType)
    return exprType;
  Type *t = cc->getVariableType(name);
  if (!t) {
    printf("Undefined variable %s at line %d\n", name->c_str(), lineno);
    exit(1);
  }
  exprType = t->clone();
  return exprType;
}
