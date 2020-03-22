#pragma once
#include "../Statement.h"
#include "FunctionSignature.h"
#include "StructStatement.h"
#include <map>
#include <string>
#include <vector>

class InterfaceStatement : public Statement {
public:
  std::string *name;
  std::vector<FunctionSignature *> *members;

  std::map<std::string, StructStatement *> implements;

  InterfaceStatement(std::string *n, std::vector<Statement *> *m);

  virtual ~InterfaceStatement();
  virtual void codegen(Context *cc);
  virtual void compile(Context *cc);
};
