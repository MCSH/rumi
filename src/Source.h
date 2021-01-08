#pragma once

#include "parser/parser.h"

#include <sstream>
#include <string>
#include <vector>
#include <map>

struct ParseState{
  Token * token = 0;
  bool binOp = false;
  Token *binToken = 0;
};

class Source {
public:
  std::string name;
  std::stringstream sstr;
  std::string str;
  std::map<int, ParseState> mem;
  Source(std::string name) : name(name) {}
  void loadBuff();
  void fetch();
  ParseState *resolveState(int pos);
};

std::ostream &operator<<(std::ostream &os, const Source &s);
