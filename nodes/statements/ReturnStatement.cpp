#include "ReturnStatement.h"
#include "../../Context.h"

void ReturnStatement::compile(CompileContext *cc) {
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

void ReturnStatement::codegen(CodegenContext *cc) {
  // TODO check void
  if (exp) {
    cc->builder->CreateStore(exp->exprGen(cc), cc->getReturnAlloca());
  }
  // No special thing needed for void
  cc->builder->CreateBr(cc->getEndBlock());
}
