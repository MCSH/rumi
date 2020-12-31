#pragma once

#include <sstream>
#include <string>
#include <vector>
#include <map>
#include "parser/parser.h"

#define endl std::endl;

void printInfo();

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

enum Verbosity {
  NONE = 0,
  LOW = 1,
  MEDIUM = 2,
  HIGH = 3,
};

class CompileContext {
public:
  std::vector<Source *> sources;
  int verbosity;
  Parser parser;
  std::ostream &debug(int v);
  void load(Source *s);
 CompileContext(int argc, char **argv);
};

using CC = CompileContext;
