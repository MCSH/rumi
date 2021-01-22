#pragma once
#include "../CC.h"
#include <set>

class AST{
 public:
  virtual void compile(CC *cc) = 0; // identify the link dependency
  virtual void prepare(CC *cc) = 0; // identify the compile dependency

  virtual void add(std::string key, void *value) = 0; // add to list of key
  virtual void set(std::string key, void *value) = 0; // set to key

  virtual ~AST() = 0;
};
