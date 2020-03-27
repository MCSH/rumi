#include "MemberStatement.h"
#include "../../Context.h"
#include "StructStatement.h"
#include "../types/StructType.h"
#include "../types/PointerType.h"

MemberStatement::~MemberStatement() {
  delete name;
  delete f;
}

void MemberStatement::codegen(Context *cc){
  f->codegen(cc);
}

void MemberStatement::compile(Context *cc) {
  // Resolve the struct,
  // Add to it's methods
  MemberStatement *m = this; // TODO lazy
  StructStatement *s = cc->getStruct(m->name);
  if (!s) {
    printf("Struct %s not found on line %d\n", m->name->c_str(), m->lineno);
    exit(1);
  }
  std::string methodName(*m->f->sign->name);
  *m->f->sign->name = (*m->name) + "$" + (*m->f->sign->name);
  Type *selfT = new StructType(new std::string(m->name->c_str()));
  selfT = new PointerType(selfT);
  ArgDecl *self = new ArgDecl(new std::string("self"), selfT, false);
  m->f->sign->args->insert(m->f->sign->args->begin(), self);
  m->f->compile(cc);
  s->methods[methodName] = m->f;
}
