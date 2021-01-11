#pragma once
#include <string>

class Type;

class Named{
 public:
  std::string id;
  Type *type = 0;
  void *alloca = 0;
  bool isType = false;
};

