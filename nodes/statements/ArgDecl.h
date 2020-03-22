#pragma once
#include "VariableDecl.h"

class ArgDecl: public VariableDecl{
public:
  std::string *name;
  Type *t;
  bool vardiac;
  ArgDecl(std::string *n, Type *t, bool v=false): name(n), t(t), vardiac(v){
  }

  virtual ~ArgDecl(){
    delete name;
    delete t;
  }
  virtual void compile(Context *cc){}
};
