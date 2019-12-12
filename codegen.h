#pragma once
#include "node.h"

class CompilerContext{
};

CompilerContext* codegen(std::vector<Statement *> *statements);
