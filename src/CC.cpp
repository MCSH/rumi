#include "base.h"
#include "Source.h"
#include <fstream>
#include <iostream>

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

CompileContext::CompileContext(int argc, char **argv){
  this->verbosity = 0;
  for (int i = 1; i < argc; i++) {
    std::string arg = std::string(argv[i]);
    if (arg == "-v") {
      this->verbosity++;
    } else if(arg == "-o"){
      // TODO make sure this is safe
      this->outFile = std::string(argv[i+1]);
      i++;
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


std::ostream &CompileContext::debug(int v) {
  if (v <= this->verbosity) {
    return std::cout;
  }
  static NullStream ns;
  return ns;
}

void CompileContext::load(Source *s) {
  s->loadBuff();
  s->fetch();
  this->debug(LOW) << s->str << std::endl;
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

  debug(HIGH) << "Registering " << id << std::endl;
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

std::string CompileContext::pathResolve(std::string path){
  if(path.find(".rum") == std::string::npos){
    path += ".rum";
  }
  return path;
}
