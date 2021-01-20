#pragma once

#include "parser/parser.h"
#include <map>
#include <ostream>

void printInfo();

class Source;
class LLContext;
class Named;

enum Verbosity {
  NONE = 0,
  LOW = 1,
  MEDIUM = 2,
  HIGH = 3,
};

class BlockContext{
public:
  BlockContext *parent = 0;
  std::map<std::string, Named *> named;
};

class CompileContext {
public:
  std::vector<Source *> sources;
  int verbosity;
  Parser parser;
  LLContext *llc;
  std::ostream &debug(int v);
  std::string outFile = "out.o";
  void load(Source *s);
  CompileContext(int argc, char **argv);

  BlockContext *block = new BlockContext();

  void pushContext();
  void pushContext(BlockContext *b);
  BlockContext *popContext();

  void registerNamed(std::string id, Named *n);
  Named *lookup(std::string id);

  std::string pathResolve(std::string path);
};

using CC = CompileContext;
