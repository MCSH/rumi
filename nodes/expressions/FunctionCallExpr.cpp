#include "FunctionCallExpr.h"
#include "../../Context.h"
#include "../../codegen.h"
#include "../statements/ArgDecl.h"
#include "../types/ArrayType.h"
#include "../types/FunctionType.h"
#include "../types/VoidType.h"

FunctionCallExpr::FunctionCallExpr(std::string *n,
                                   std::vector<Expression *> *expr)
    : name(n), expr(expr) {
  if (!expr)
    this->expr = new std::vector<Expression *>();
}

FunctionCallExpr::~FunctionCallExpr() {
  delete name;
  for (auto e : *expr)
    delete e;
  delete expr;
}

llvm::Value *FunctionCallExpr::exprGen(Context *cc) {
  // TODO args, function resolve could be improved!
  llvm::Function *calleeF = cc->module->getFunction(this->name->c_str());
  llvm::Value *cf;
  bool is_void;
  if (!calleeF) {
    // maybe it's a function variable
    auto vd = cc->getVariableDecl(this->name);
    ArgDecl *ad;

    if (vd && (ad = dynamic_cast<ArgDecl *>(vd)) &&
        typeid(*ad->t).hash_code() == typeid(FunctionType).hash_code()) {
      auto cd = cc->getVariableAlloca(this->name);
      cf = cc->builder->CreateLoad(cd);
      is_void = dynamic_cast<VoidType *>(((FunctionType *)ad->t)->returnType);
    } else if (vd &&
               typeid(*vd->t).hash_code() == typeid(FunctionType).hash_code()) {
      // it's a function variable
      auto cd = cc->getVariableAlloca(this->name);
      cf = cc->builder->CreateLoad(cd);
      is_void = dynamic_cast<VoidType *>(((FunctionType *)vd->t)->returnType);
    } else {
      printf("Function not found %s\n", this->name->c_str());
      printf("line no %d\n", this->lineno);
      printf("This is a compiler bug, reported from "
             "codegen::functionCallExprGen\n");
      exit(1);
    }
  } else {
    is_void = calleeF->getReturnType()->isVoidTy();
    cf = calleeF;
  }

  // There is another function call in the interface method call function
  std::vector<llvm::Value *> argsV;
  for (auto e : *this->expr) {
    if (e->exprType &&
        typeid(*e->exprType).hash_code() == typeid(ArrayType).hash_code()) {
      // For Arrays, send the pointer to the first element

      ArrayType *at = (ArrayType *)e->exprType;

      auto t = at->typeGen(cc);
      auto alloc = e->getAlloca(cc);

      if (at->count) {
        argsV.push_back(arrayToPointer(t, alloc, cc));
      } else if (at->exp) {
        argsV.push_back(alloc);
      } else {
        argsV.push_back(cc->builder->CreateLoad(alloc));
      }
    } else {
      argsV.push_back(e->exprGen(cc));
    }
  }

  if (is_void)
    return cc->builder->CreateCall(cf, argsV);
  return cc->builder->CreateCall(cf, argsV, "calltmp");
}

llvm::Value *FunctionCallExpr::getAlloca(Context *cc) {}

void FunctionCallExpr::codegen(Context *cc) { exprGen(cc); }

void FunctionCallExpr::compile(Context *cc) {
  resolveType(cc);
  auto fce = this; // TODO lazy

  // check function exists
  auto fs = cc->getFunction(fce->name);
  if (!fs) {
    // it could be a function variable
    auto fv = cc->getVariableType(fce->name);

    if (!fv || typeid(*fv).hash_code() != typeid(FunctionType).hash_code()) {
      printf("Function not found %s\n", fce->name->c_str());
      exit(1);
    }
  }

  // generate arguments
  if (!fce->expr)
    fce->expr = new std::vector<Expression *>();

  for (auto e : *fce->expr)
    e->compile(cc);
}

Type *FunctionCallExpr::resolveType(Context *cc) {
  if (exprType)
    return exprType;
  auto f = cc->getFunction(name);
  if (!f) {
    // maybe it's a function variable
    auto f = cc->getVariableType(name);

    // verify it is of function type:
    if (f && typeid(*f).hash_code() == typeid(FunctionType).hash_code()) {
      // It's okay
      exprType = ((FunctionType *)f)->returnType->clone();
      return exprType;
    }

    printf("Unknown function %s at line %d\n", name->c_str(), lineno);
    exit(1);
  }
  exprType = f->returnT->clone();
  return exprType;
}
