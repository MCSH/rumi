#include "Function.h"
#include "Statement.h"

void Function::prepeare(CC *cc){
  for(Statement *s: statements){
    for(AST *dep: s->compileDeps){
      compileDeps.insert(dep);
    }
  }

  // TODO args, returnType

  for(Statement *s: statements){
    s->prepeare(cc);
  }
}

void Function::compile(CC *cc){
  for(Statement *s: statements){
    for(AST *dep: s->linkDeps){
      linkDeps.insert(dep);
    }
  }
  // TODO args, returnType
}


void Function::codegen(CC *cc){
  // TODO
}
