#pragma once
#include "Statement.h"
#include "../Source.h"

class Import: public Statement{
public:
  std::string importPath;
  Source *s;

  Import(std::string importPath);

  virtual void set(std::string key, void *value) override;
  virtual void add(std::string key, void *value) override;

  virtual void prepare(CC *cc) override;
  virtual void compile(CC *cc) override;
  virtual void codegen(CC *cc) override;
};
