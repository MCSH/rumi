#pragma once
#include "../CC.h"

class Token {
public:
  virtual std::string desc() = 0;
  int spos;
  int epos;

  // hack to allow operator
  Source *s;
  CC *cc;
};

std::ostream &operator<<(std::ostream &os, Token &s);
