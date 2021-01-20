#pragma once

#include "parser/parser.h"

#include <sstream>
#include <string>
#include <vector>
#include <map>

struct ParseState{
  Token * token = 0;
  std::map<std::string, Token *> memoizations;

  bool hasParser(std::string name);
  Token *getToken(std::string name);
  void setToken(std::string name, Token * token);
};

class Source {
public:
  std::string name;
  std::stringstream sstr;
  std::string str;
  std::map<int, ParseState*> mem;
  Source(std::string name) : name(name) {}
  void loadBuff();
  void fetch();
  ParseState *resolveState(int pos);

  std::vector<AST *> *parse(CC *cc);
};

std::ostream &operator<<(std::ostream &os, const Source &s);
