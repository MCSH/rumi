#pragma once

#include "parser/parser.h"

void printInfo();

class Source;

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
