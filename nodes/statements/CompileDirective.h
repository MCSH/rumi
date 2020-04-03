#pragma once
#include "FunctionDefine.h"
#include <llvm/IR/Function.h>
#include <string>


class CompileDirective {
  //std::string *name;
  llvm::Function *f;
  FunctionDefine *df;
  int lineno;

 public:
  CompileDirective(llvm::Function *f, FunctionDefine *df);
  ~CompileDirective();
  void run(Context *cc, Statement *stmt);
};
