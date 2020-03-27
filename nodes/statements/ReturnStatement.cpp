#include "ReturnStatement.h"
#include "../../Context.h"
#include "../../compiler.h"
#include "FunctionDefine.h"

void ReturnStatement::compile(Context *cc) {
  // TODO handle void?

  if (!exp)
    return;

  exp->compile(cc);

  // return type
  Type *t = exp->resolveType(cc);
  Type *ft = cc->getCurrentFunction()->sign->returnT;

  // check the return type using castcompile
  exp = castCompile(t, ft, exp, cc, this, false);
}

void ReturnStatement::codegen(Context *cc) {
  // TODO check void
  if (exp) {
    cc->builder->CreateStore(exp->exprGen(cc), cc->getReturnAlloca());
  }
  // No special thing needed for void
  cc->builder->CreateBr(cc->getEndBlock());
}
