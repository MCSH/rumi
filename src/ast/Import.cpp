#include "Import.h"
#include "../base.h"

Import::Import(std::string importPath)
  : importPath(importPath)
{
}

void Import::prepare(CC *cc){
  s = new Source(cc->pathResolve(importPath));
  asts = *s->parse(cc);

  for(AST *a: asts){
    a->prepare(cc);
  }

}
void Import::compile(CC *cc){
  for(AST *a: asts){
    a->compile(cc);
  }
}
void Import::codegen(CC *cc){
  for(AST *a: asts){
    if(Statement *s = dynamic_cast<Statement *>(a))
      s->codegen(cc);
  }
}
