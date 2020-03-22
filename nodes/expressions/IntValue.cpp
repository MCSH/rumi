#include "IntValue.h"
#include <llvm/IR/Constant.h>
#include "../../Context.h"
#include "../types/IntType.h"

IntValue::~IntValue() {
  if (val)
    delete val;
}
llvm::Value *IntValue::exprGen(Context *cc){
  // TODO Check max size
  int value = this->size;
  if(this->val)
    value = atoi(this->val->c_str());
  // TODO check params?
  return llvm::ConstantInt::get(llvm::Type::getInt64Ty(cc->context), value, true);
}

llvm::Value *IntValue::getAlloca(Context *cc) {
  printf("Invalid operation, can't assign to int\n");
  exit(1);
}

void IntValue::compile(Context *cc){
  resolveType(cc);
  // TODO check max size
  // TODO should we move atoi here?
}

Type *IntValue::resolveType(Context *cc){
  if(exprType)
    return exprType;
  exprType = new IntType();
  return exprType;
}
