#include "Source.h"
#include "CC.h"
#include "ast/Statement.h"
#include "ast/ast.h"
#include "base.h"
#include <fstream>
#include "DebugInfo.h"

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

void Source::parse(CC *cc){
  if(cc->loaded.find(this->name) != cc->loaded.end()){
    return; // Already loaded this file
  }
  cc->loaded.insert(this->name);
  cc->load(this);

  ParseResult t;
  t = cc->parser.parseTop(this);
  int epos = 0;
  while(t){
    epos = t.token->epos;
    AST *a = t.token->getAST(cc);
    if(a){
      // ans->push_back(a);
      cc->asts.push_back(a);
      a->prepare(cc);
      a->compile(cc);
    }
    if(false && a){
      a->prepare(cc);
      a->compile(cc);
      if (Statement *s = dynamic_cast<Statement *>(a))
        s->codegen(cc);
    }
    cc->debug(Verbosity::MEDIUM) << t << std::endl;
    t = cc->parser.parseTop(this, t.token->epos + 1);
  }

  if (skipwscomment(&this->str, epos + 1) != -1) {
    graceFulExit(new DebugInfo(&t), "Couldn't parse file");
    //cc->debug(NONE) << "Couldn't parse file at index " << epos << " - " <<  str.size() << std::endl;
    //exit(1);
  }
}

