#include "Import.h"
#include "../base.h"

Import::Import(std::string importPath)
  : importPath(importPath)
{
}

void Import::prepare(CC *cc){
  s = new Source(cc->pathResolve(importPath));
  s->parse(cc);
}
void Import::compile(CC *cc){
}
void Import::codegen(CC *cc){
}
