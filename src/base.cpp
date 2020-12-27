#include "base.h"
#include "RumiConfig.h"
#include <fstream>
#include <iostream>
#include <streambuf>

void printInfo() {
  std::cout << "Rumi Version " << Rumi_VERSION_MAJOR << "."
            << Rumi_VERSION_MINOR << endl;
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
      << "Verbosity set at " << this->verbosity << endl;
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
  this->debug(LOW) << s->str << endl;
}

void Source::loadBuff() {
  std::ifstream f(this->name);

  this->sstr << f.rdbuf();
}
