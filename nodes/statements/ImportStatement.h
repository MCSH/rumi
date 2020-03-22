#pragma once
#include "../Statement.h"
#include <string>
#include <vector>

class ImportStatement: public Statement{
public:
  std::string *name;
  std::vector<Statement *> *stmts;
  ImportStatement(std::string *n): name(n){}
  virtual ~ImportStatement();
  virtual void codegen(Context *cc);
  virtual void compile(Context *cc);
};
