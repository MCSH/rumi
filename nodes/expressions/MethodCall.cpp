#include "MethodCall.h"
#include "../../Context.h"
#include "../expressions/PointerAccessExpr.h"
#include "../expressions/PointerExpr.h"
#include "../statements/InterfaceStatement.h"
#include "../types/ArrayType.h"
#include "../types/FunctionType.h"
#include "../types/IntType.h"
#include "../types/InterfaceType.h"
#include "../types/StructType.h"
#include "../types/VoidType.h"

llvm::Value *interfaceMethodCall(MethodCall *mce, CodegenContext *cc) {

  llvm::Value *i = mce->e->getAlloca(cc);

  // find the method index
  InterfaceType *it = (InterfaceType *)mce->e->exprType;
  InterfaceStatement *is = cc->getInterfaceStatement(it->name);

  int ind = 0;

  // We are sure that it exists because otherwise compile would raise an error
  FunctionSignature *ff;
  for (ind; ind < is->members->size(); ind++) {
    auto m = (*is->members)[ind];
    if (m->name->compare(*mce->name) == 0) {
      // we found the match
      ff = m;
      break;
    }
  }

  // get the vtable
  llvm::Type *iv = cc->getInterfaceType(it->name);
  llvm::Type *vt = cc->getInterfaceVtableType(it->name);

  std::vector<llvm::Value *> indices(2);
  indices[0] = llvm::ConstantInt::get(cc->context, llvm::APInt(32, 0, true));
  indices[1] = llvm::ConstantInt::get(cc->context, llvm::APInt(32, 0, true));
  llvm::Value *vtable =
      cc->builder->CreateInBoundsGEP(iv, i, indices, "vtable");

  // get the method pointer
  indices[1] = llvm::ConstantInt::get(cc->context, llvm::APInt(32, ind, true));
  llvm::Value *f = cc->builder->CreateInBoundsGEP(
      vt, cc->builder->CreateLoad(vtable), indices, "fptr");

  // I think we should cast f

  // change the function type when calling it
  FunctionType *ft = (FunctionType *)ff->fType->clone();

  // when calling the function, pass the pointer not the whole object

  ft->args->insert(ft->args->begin(), new PointerType(new IntType(64)));

  f = cc->builder->CreateBitCast(f, ft->typeGen(cc)->getPointerTo());

  // Replace the mce->expr[0] with the pointer to the struct

  indices[1] = llvm::ConstantInt::get(cc->context, llvm::APInt(32, 1, true));
  llvm::Value *sptr = cc->builder->CreateInBoundsGEP(iv, i, indices, "sptr");

  // call the method

  f = cc->builder->CreateLoad(f);

  std::vector<llvm::Value *> argsV;
  argsV.push_back(cc->builder->CreateLoad(sptr));

  mce->expr->erase(mce->expr->begin());

  for (auto e : *mce->expr) {
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

  bool is_void = dynamic_cast<VoidType *>(ff->returnT);
  if (is_void) {
    cc->builder->CreateCall(f, argsV);
    return 0;
  }
  return cc->builder->CreateCall(f, argsV, "calltmp");
}

MethodCall::MethodCall(Expression *e, std::string *n,
                       std::vector<Expression *> *expr)
    : e(e), name(n), expr(expr) {
  if (!expr) {
    this->expr = new std::vector<Expression *>();
  }
}

MethodCall::~MethodCall() {
  delete e;
  delete name;
  for (auto e : *expr)
    delete e;
  delete expr;
}
llvm::Value *MethodCall::exprGen(CodegenContext *cc) {
  if (f)
    return fce->exprGen(cc);
  return interfaceMethodCall(this, cc);
}

llvm::Value *MethodCall::getAlloca(CodegenContext *cc) {}

void MethodCall::codegen(CodegenContext *cc) {
  if (f)
    return fce->codegen(cc);
  interfaceMethodCall(this, cc);
}

void MethodCall::compile(CompileContext *cc) {
  resolveType(cc);
  // it's basically a function call
  MethodCall *mc = this;

  Expression *e = mc->e;

  Type *tmpe = e->exprType;

  while (PointerType *p = dynamic_cast<PointerType *>(tmpe)) {
    tmpe = p->base;
    e = new PointerAccessExpr(e);
  }

  e->compile(cc);
  mc->expr->begin();
  mc->expr->insert(mc->expr->begin(), new PointerExpr(e));
  (*mc->expr)[0]->compile(cc);
  if (mc->f) {
    // This is a method call on structs
    mc->fce = new FunctionCallExpr(mc->f->sign->name, mc->expr);
  } else {
    // This is a method call on interfaces, we have to handle in codegen
  }
}

Type *MethodCall::resolveType(CompileContext *cc) {
  if (exprType)
    return exprType;
  MethodCall *mc = this; // TODO because I'm lazy
  e->compile(cc);

  auto tmpe = mc->e->resolveType(cc);

  while (PointerType *p = dynamic_cast<PointerType *>(tmpe)) {
    tmpe = p->base;
  }

  auto t = (StructType *)tmpe;

  auto st = cc->getStruct(t->name);

  // TODO it might be an interface

  if (!st) {
    auto it = cc->getInterface(t->name);
    if (!it) {
      // This will never happen
      printf("Unknown type %s on line %d\n", t->name, mc->lineno);
      printf("This should not happen\n");
      exit(1);
    }
    int ind = 0;
    for (auto f : *it->members) {
      if (f->name->compare(*mc->name) == 0) {
        mc->f = 0;
        mc->methodInd = ind;
        return f->returnT;
      }
      ind++;
    }

    printf("Interface %s doesn't have any method named %s, on line %d\n",
           t->name->c_str(), mc->name->c_str(), mc->lineno);
    exit(1);
  }

  FunctionDefine *f = st->methods[*mc->name];

  if (!f) {
    printf("Struct %s doesn't have any method named %s, on line %d\n",
           t->name->c_str(), mc->name->c_str(), mc->lineno);
    exit(1);
  }

  mc->f = f;

  return f->sign->returnT;
}
