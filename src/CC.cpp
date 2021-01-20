#include "LLContext.h"
#include "base.h"
#include "Source.h"
#include <fstream>
#include <iostream>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/IRBuilder.h>

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

void exitCallback(void *c, int i){
  CC *cc = (CC *) c;
  exit(i);
}

void setCallback(void *c, char *ckey, void *value){
  // TODO
  CC *cc = (CC *) c;
  std::string key(ckey);

  if(key == "verbosity"){
    // TODO
    int v = (long long) value;
    cc->verbosity = v;
  } else {
    cc->debug(NONE) << "Compiler key " << key << " not found." << std::endl;
    exit(1);
  }
}

void *getCallback(void *c, char *ckey){
  CC *cc = (CC *) c;
  std::string key(ckey);

  if(key == "verbosity"){
    return (void *)(long long)cc->verbosity;
  } else {
    cc->debug(NONE) << "Compiler key " << key << " not found." << std::endl;
    exit(1);
  }
}

#define REGISTER_CALLBACK(name, replace, cb)                                   \
  {                                                                            \
    llvm::Function *f = EE->FindFunctionNamed(name);                           \
    llvm::Function *n = llvm::Function::Create(                                \
        f->getFunctionType(), llvm::Function::ExternalLinkage, replace,        \
        *this->llc->module);                                                   \
    auto bb = llvm::BasicBlock::Create(this->llc->context, "entry", n);        \
    llvm::IRBuilder<> builder(bb, bb->begin());                                \
    builder.SetInsertPoint(bb);                                                \
    auto fp = builder.CreateIntToPtr(                                          \
        llvm::ConstantInt::get(llvm::Type::getInt64Ty(llc->context),           \
                               (uint64_t)cb, false),                           \
        f->getFunctionType()->getPointerTo());                                 \
    std::vector<llvm::Value *> *args;                                          \
    args = new std::vector<llvm::Value *>();                                   \
    auto inargs = n->args();                                                   \
    for (auto &arg : inargs)                                                   \
      args->push_back(&arg);                                                   \
    builder.CreateRet(builder.CreateCall(f->getFunctionType(), fp, *args));    \
    f->replaceAllUsesWith(n);                                                  \
    n->takeName(f);                                                            \
    f->eraseFromParent();                                                      \
  }

void *CompileContext::getCompileObj(void *e){

  llvm::ExecutionEngine *EE = (llvm::ExecutionEngine*) e;
  
  if(compileObj) return compileObj;

  compileObj = this; // nothign is insie, so we don't care

  // Initialize compile functions in the module.

  // == compiler$exit := (c: compiler, status: int)-> unit ==
  REGISTER_CALLBACK ("compiler$exit", "compiler$exit_replace", &exitCallback)
  REGISTER_CALLBACK ("compiler$set", "compiler$set_replace", &setCallback)
  REGISTER_CALLBACK ("compiler$get", "compiler$get_replace", &getCallback)

  return compileObj;
}
