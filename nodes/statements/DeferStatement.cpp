#include "DeferStatement.h"
#include "../../Context.h"

DeferStatement::~DeferStatement() { delete s; }

void DeferStatement::codegen(Context *cc){
  cc->defered.back()->push_back(s);
}

void DeferStatement::compile(Context *cc){
  // TODO Maybe we should handle defer here instead of codegen?
  s->compile(cc);
}
