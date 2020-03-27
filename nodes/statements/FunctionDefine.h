#pragma once
#include "../Statement.h"
#include "FunctionSignature.h"
#include "../../BlockContext.h"

// This prevents recursive inclusion
class CodeBlock;

class FunctionDefine: public Statement, public BlockContext{
public:
  FunctionSignature *sign;
  CodeBlock *body;
  bool dynamicStack = false; // Wether we have dynamic stack, i.e., stack array of dynamic size
  FunctionDefine(FunctionSignature *d, CodeBlock *b): sign(d), body(b){
  }

  virtual ~FunctionDefine();
  llvm::Function *funcgen(Context *cc);
  virtual void codegen(Context *cc);
  virtual void compile(Context *cc);
};
