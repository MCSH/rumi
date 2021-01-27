#include "PackedAST.h"
#include "../base.h"

void PackedAST::prepare(CC *cc){
  for(AST *s: stmts)
    s->prepare(cc);
}

void PackedAST::compile(CC *cc){
  for(AST *s: stmts)
    s->compile(cc);
}

void PackedAST::codegen(CC *cc){
  for(AST *a: stmts)
    if(auto s = dynamic_cast<Statement *>(a))
      s->codegen(cc);
}
