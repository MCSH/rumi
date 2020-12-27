#pragma once
#include <string>

class CompileContext;
using CC = CompileContext;
class Source;

enum Keyword: short{
  k_ret,
  k_if,
  k_while,
};

enum Symbol: short{
  s_plus,
  s_semicolon,
};
