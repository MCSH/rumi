#include "Source.h"
#include "base.h"
#include <fstream>

std::ostream &operator<<(std::ostream &os, const Source &s) {
  os << s.name;
  return os;
}

void Source::fetch(){
  str = sstr.str();
}

void Source::loadBuff() {
  std::ifstream f(this->name);

  this->sstr << f.rdbuf();
}

ParseState *Source::resolveState(int pos){
  auto it = mem.find(pos);
  if(it != mem.end()){
    return (*it).second;
  }
  /// Create it
  mem[pos] = new ParseState();
  return mem[pos];
}

bool ParseState::hasParser(std::string name){
  auto it = memoizations.find(name);
  if(it != memoizations.end()){
    return true;
  }
  return false;
}

void ParseState::setToken(std::string name, Token * token){
  memoizations[name] = token;
}

Token *ParseState::getToken(std::string name){
  return memoizations[name];
}

std::vector<AST *> *Source::parse(CC *cc){
  auto ans = new std::vector<AST *>();

  cc->load(this);

  ParseResult t;
  t = cc->parser.parseTop(this);
  int epos = 0;
  while(t){
    epos = t.token->epos;
    AST *a = t.token->toAST(cc);
    if(a) ans->push_back(a);
    cc->debug(Verbosity::MEDIUM) << t << std::endl;
    t = cc->parser.parseTop(this, t.token->epos + 1);
  }

  if (skipwscomment(&this->str, epos + 1) != -1) {
    cc->debug(NONE) << "Couldn't parse file at index " << epos << " - " <<  str.size() << std::endl;
    exit(1);
  }
  return ans;
}

