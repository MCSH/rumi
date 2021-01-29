#pragma once
#include <ostream>
#include "CC.h"

class Token;
class Failure;
class ParseResult;

class DebugInfo{
  // gathered
  int pos;
  Source *s;
  CC *cc;

  // processed
  int lineno;
  bool unknown = false;

  void fromToken(Token *t);
  void fromFailure(Failure *f);

  void resolvelineno();
  
 public:
  DebugInfo(Token *t);
  DebugInfo(Failure *t);
  DebugInfo(ParseResult *p);

  virtual std::string toString();
};


std::ostream &operator<<(std::ostream &os, DebugInfo &s);
