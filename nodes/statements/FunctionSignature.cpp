#include "FunctionSignature.h"
#include "../../Context.h"
#include "../types/FunctionType.h"

// TODO no context in codegen? is this right?

FunctionSignature::FunctionSignature(std::string *val, std::vector<Statement *> *a, Type *t)
    : name(val), returnT(t) {
  args = new std::vector<ArgDecl *>();
  if (a) {
    for (auto s : *a) {
      args->push_back((ArgDecl *)s);
    }
    delete a;
  }
}

Type *FunctionSignature::getType(){
  // TODO maybe cache this?
  if(fType)
    return fType;

  std::vector<Type *> *a=new std::vector<Type*>();

  for(auto arg: *args){
    a->push_back(arg->t->clone());
  }

  fType = new FunctionType(a, returnT->clone());
  return fType;
}

FunctionSignature::~FunctionSignature() {
  delete name;
  delete returnT;

  for (auto s : *args) {
    delete s;
  }
  delete args;
}
void FunctionSignature::codegen(Context *cc){
  signgen(cc);
}

llvm::Function *FunctionSignature::signgen(Context *cc){
  // TODO
  auto fargs = this->args;
  std::vector<llvm::Type *> args;
  bool has_varargs = false;
  for(auto arg: *fargs){
    if(arg->vardiac){
      has_varargs = arg->vardiac; // Only the last one matters.
      break;
    }
    args.push_back(arg->t->typeGen(cc));
  }
  // TODO vargs, somehow

  auto type = this->returnT->typeGen(cc);

  llvm::FunctionType *fT = llvm::FunctionType::get(type, args, has_varargs);

  llvm::Function* f;

  if(this->isLocal){
    // TODO choose the best linkage
    f = llvm::Function::Create(fT, llvm::Function::PrivateLinkage, *this->name, *cc->module);
  } else {
    f = llvm::Function::Create(fT, llvm::Function::ExternalLinkage, *this->name, *cc->module);
  }

  return f;
}


void FunctionSignature::noBlockCompile(Context *cc){
  auto fs = this; // TODO lazy
  BlockContext *b = cc->getBlock();

  bool varargs = false;
  for(auto arg: *fs->args){
    if(varargs){
      // if this is true, a vardiac variable was defined in the middle, which is an error.
      printf("Vardiac variables should only be the last ones.\nError on line %d, function %s\n", fs->lineno, fs->name->c_str());
      exit(1);
    }
    if(!arg->vardiac)
      b->newVar(arg->name, arg->t);
    else {
      // TODO define the vardiac variable when we have arrays
      varargs = false;
    }
  }

  cc->getParentBlock()->newFunction(fs->name, fs);
}

void FunctionSignature::compile(Context *cc){
    cc->blocks.push_back(this);

    this->noBlockCompile(cc);

    cc->blocks.pop_back();
    return;
}

