#pragma once
#include "Statement.h"
#include "../Source.h"

class Import: public Statement{
public:
  std::string importPath;
  Source *s;

  Import(std::string importPath);

  virtual void prepare(CC *cc) override;
  virtual void compile(CC *cc) override;
  virtual void codegen(CC *cc) override;
};
