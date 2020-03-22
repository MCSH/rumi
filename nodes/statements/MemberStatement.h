#pragma once
#include "../Statement.h""
#include "FunctionDefine.h"
#include <string>

class MemberStatement: public Statement{
public:
  std::string *name;
  FunctionDefine *f;
  MemberStatement(std::string *n, Statement *f): name(n), f((FunctionDefine*)f){
    // TODO Change the function name to prevent name colision
  }

  virtual ~MemberStatement();
  virtual void codegen(Context *cc);
  virtual void compile(Context *cc);
};
