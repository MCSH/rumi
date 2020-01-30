#pragma once
#include "../Statement.h"
#include "VariableDecl.h"
#include "FunctionDefine.h"
#include <string>
#include <vector>
#include <map>

class StructStatement: public Statement{
public:
  std::string *name;
  std::vector<VariableDecl *> *members;
  std::map<std::string, FunctionDefine *> methods;
  bool has_initializer = false;
  int type_counter;
  StructStatement(std::string *name, std::vector<Statement *> *m);

  virtual ~StructStatement();
  virtual void codegen(CodegenContext *cc);
  virtual void compile(CompileContext *cc);
};
