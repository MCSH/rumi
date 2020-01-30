#pragma once
#include "../Statement.h"
#include "ArgDecl.h"
#include <string>
#include <vector>

class FunctionSignature : public Statement {
public:
  std::string *name;
  Type *returnT;
  Type *fType = 0;
  std::vector<ArgDecl *> *args;
  bool isLocal = false;

  FunctionSignature(std::string *val, std::vector<Statement *> *a, Type *t);
  void noBlockCompile(CompileContext *cc);
  Type *getType();
  virtual ~FunctionSignature();
  virtual void codegen(CodegenContext *cc);
  llvm::Function *signgen(CodegenContext *cc);
  virtual void compile(CompileContext *cc);
};
