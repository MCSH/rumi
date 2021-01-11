#include "Struct.h"
#include "../base.h"
#include "../LLContext.h"
#include "Named.h"
#include "Type.h"

StructType::StructType(std::string id)
  : id(id)
{}


void StructType::prepare(CC *cc){
  // TODO
  // TODO check for name clashes
  Named *named = new Named();
  named->id = id;
  named->type = this;
  named->alloca = 0;
  named->isType = true;
  cc->registerNamed(id, named);
}

void StructType::compile(CC *cc){
  // TODO do I want anything here?
  // TODO make sure types are valid
}

void *StructType::typegen(CC *cc){
  // TODO
  if(generatedType) return generatedType;

  std::vector<llvm::Type *> mem_t;
  auto s = llvm::StructType::create(cc->llc->context, mem_t, id);

  for(auto m: members){
    mem_t.push_back((llvm::Type *)m->type->typegen(cc));
  }

  s->setBody(mem_t);

  generatedType = s;

  cc->lookup(id)->alloca = s;

  return s;
}

bool StructType::hasOp(CC *cc, std::string op, Expression *rhs){
  // TODO
  return false;
}
void* StructType::opgen(CC *cc, Expression *lhs, std::string op, Expression *rhs){
  // TODO
  return 0;
}
Type* StructType::optyperesolve(CC *cc, std::string op, Expression *rhs){
  // TODO
  return 0;
}
Compatibility StructType::compatible(Type *t){
  // TODO
  return INCOMPATIBLE;
}
void* StructType::castgen(CC *cc, Expression *e){
  // TODO
  return 0;
}
