#include "BlockContext.h"
#include "nodes/statements/FunctionSignature.h"

void BlockContext::newFunction(std::string *name, FunctionSignature *fs) {
  // TODO check for name collision
  // TODO maybe register as var?
  functions[*name] = fs;
  newVar(name, fs->getType());
}

BlockContext::~BlockContext(){}
