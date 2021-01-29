#include "../DebugInfo.h"
#include <algorithm>
#include <sstream>
#include "Token.h"
#include "../Source.h"

void DebugInfo::resolvelineno(){
  lineno = std::count(s->str.begin(), s->str.begin() + pos, '\n') + 1;
}

void DebugInfo::fromToken(Token *t){
  pos = t->spos;
  s = t->s;
  cc = t->cc;

  resolvelineno();
}

void DebugInfo::fromFailure(Failure *f){
  pos = f->pos;
  s = f->s;
  cc = f->cc;

  resolvelineno();
}

DebugInfo::DebugInfo(Token *t){
  fromToken(t);
}

DebugInfo::DebugInfo(Failure *f){
  fromFailure(f);
}

DebugInfo::DebugInfo(ParseResult *p){
  if(p){
    fromToken(p->token);
  } else if(p->failure){
    fromFailure(p->failure);
  } else {
    unknown = true;
  }
}

DebugInfo::DebugInfo(CC *cc, Source *s, int pos){
  this->cc = cc;
  this->s = s;
  this->pos = pos;

  resolvelineno();
}

std::string DebugInfo::toString(){
  if(unknown) return "unknown location";
  std::stringstream sb;
  sb << "[" << s->name << ":" << lineno <<"]";
  return sb.str();
}

std::ostream &operator<<(std::ostream &os, DebugInfo &d){
  return os << d.toString();
}
