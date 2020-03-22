#include "StructStatement.h"
#include "../../Context.h"

StructStatement::StructStatement(std::string *name, std::vector<Statement *> *m)
    : name(name) {
  members = new std::vector<VariableDecl *>();
  for (auto s : *m)
    members->push_back((VariableDecl *)s);
}

StructStatement::~StructStatement() {
  delete name;
  for (auto s : *members)
    delete s;
  delete members;
}

void StructStatement::codegen(Context *cc){
  // TODO

  this->type_counter = cc->struct_type_counter;
  cc->struct_type_counter++;

  std::vector<llvm::Type *> members_t;

  std::vector<llvm::Type *> vptr_t;
  // First element is the class number

  vptr_t.push_back(llvm::Type::getInt64Ty(cc->context));

  auto vt = llvm::StructType::create(cc->context, vptr_t);

  // create the global vptr

  std::string vptr_name = "$$_vptr$"+ *this->name;

  cc->module->getOrInsertGlobal(vptr_name, vt);
  auto vp = cc->module->getNamedGlobal(vptr_name);
  vp->setLinkage(llvm::GlobalValue::ExternalLinkage);

  // TODO set the initializer after all methods are registered.
  std::vector<llvm::Constant *> v;
  v.push_back(llvm::ConstantInt::get(llvm::Type::getInt64Ty(cc->context),
                                     this->type_counter, false));
  vp->setInitializer(llvm::ConstantStruct::getAnon(cc->context, v, false));

  llvm::Type *vptr = vt->getPointerTo();
  members_t.push_back(vptr);

  auto s = llvm::StructType::create(cc->context, members_t, *this->name); // TODO check params

  for(auto m: *this->members){
    members_t.push_back(m->t->typeGen(cc));
  }

  s->setBody(members_t);

  // TODO set the compiler context
  cc->setStruct(this->name, s, this);
}

void StructStatement::compile(Context *cc){
  auto ss = this; // TODO lazy
  cc->getBlock()->newStruct(ss->name, ss);
  // TODO do we want to do anything with the members?
  for(VariableDecl *s: *ss->members){
    s->compile(cc);
    if(s->exp)
      ss->has_initializer = true;
  }
}
