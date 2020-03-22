#include "InterfaceStatement.h"
#include "../../Context.h"

InterfaceStatement::InterfaceStatement(std::string *n,
                                       std::vector<Statement *> *m)
    : name(n) {
  members = new std::vector<FunctionSignature *>();
  for (auto i : *m) {
    members->push_back((FunctionSignature *)i);
  }
  delete m;
}

InterfaceStatement::~InterfaceStatement() {
  delete name;
  for (auto i : *members)
    delete i;
  delete members;
}
void InterfaceStatement::codegen(Context *cc){
  // TODO

  // Create a vtable struct
  std::vector<llvm::Type *> vptr_t;

  for(auto m: *this->members){
    vptr_t.push_back(llvm::Type::getInt64PtrTy(cc->context));
  }

  auto vt = llvm::StructType::create(cc->context, vptr_t, (*this->name)+ "$_interface_vptr");

  // TODO include pointers to subinterfaces

  // create interface object
  std::vector<llvm::Type *> intf;
  intf.push_back(vt->getPointerTo());
  intf.push_back(llvm::Type::getInt64PtrTy(cc->context));

  auto it = llvm::StructType::create(cc->context, intf, (*this->name));

  cc->setInterface(this->name, it, vt, this);
}

void InterfaceStatement::compile(Context *cc){
  // Register it like a struct
  auto is = this; // TODO lazy
  for (FunctionSignature *fs : *is->members) {
    fs->compile(cc);
  }

  cc->getBlock()->newInterface(is->name, is);
}

