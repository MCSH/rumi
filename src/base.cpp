#include "base.h"
#include "Source.h"
#include "RumiConfig.h"
#include <fstream>
#include <iostream>
#include <streambuf>

void printInfo() {
  std::cout << "Rumi Version " << Rumi_VERSION_MAJOR << "."
            << Rumi_VERSION_MINOR << std::endl;
}

CompileContext::CompileContext(int argc, char **argv){
  this->verbosity = 0;
  for (int i = 1; i < argc; i++) {
    std::string arg = std::string(argv[i]);
    if (arg == "-v") {
      this->verbosity++;
    } else {
      this->sources.push_back(new Source(arg));
      // std::cout << argv[i] << std::endl;
    }
  }
  this->debug(Verbosity::LOW)
    << "Verbosity set at " << this->verbosity << std::endl;
  /// TODO PrintInfo and exit on arg parsing error
  parser.init(this);
}

// Used in outputing to nowhere.
class NullStream : public std::ostream {
public:
  NullStream() : std::ostream(nullptr) {}
  NullStream(const NullStream &) : std::ostream(nullptr) {}
};
template <class T>
const NullStream &operator<<(NullStream &&os, const T &value) {
  return os;
}

std::ostream &CompileContext::debug(int v) {
  if (v <= this->verbosity) {
    return std::cout;
  }
  static NullStream ns;
  return ns;
}

std::ostream &operator<<(std::ostream &os, const Source &s) {
  os << s.name;
  return os;
}

void Source::fetch(){
  str = sstr.str();
}

void CompileContext::load(Source *s) {
  s->loadBuff();
  s->fetch();
  this->debug(LOW) << s->str << std::endl;
}

void Source::loadBuff() {
  std::ifstream f(this->name);

  this->sstr << f.rdbuf();
}

ParseState *Source::resolveState(int pos){
  auto it = mem.find(pos);
  if(it != mem.end()){
    return &(*it).second;
  }
  /// Create it
  mem[pos] = ParseState();
  return &mem[pos];
}


void CompileContext::pushContext(){
  auto b = new BlockContext();
  b -> parent = block;
  block = b;
}

void CompileContext::pushContext(BlockContext *b){
  block = b;
}

BlockContext *CompileContext::popContext(){
  auto b = block;
  block = b->parent;
  return b;
}

void CompileContext::registerNamed(std::string id, Named *n){
  /// TODO check for overriding instances;
  block->named[id] = n;


  debug(NONE) << "Registering " << id << std::endl;
}

Named *CompileContext::lookup(std::string id){
  auto b = block;
  while(b){
    auto p = b->named.find(id);
    if(p != b->named.end()){
      return p->second;
    }

    b = b->parent;
  }

  return 0;
}
