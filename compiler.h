#pragma once

#include "Context.h"

Context *compile(char *fileName);

Expression *castCompile(Type *exprType, Type *baseType, Expression *e, Context *cc, Node *n, bool expl);

std::vector<Statement *>* array_compile(std::vector<Statement *> *stmts, Context *cc);
