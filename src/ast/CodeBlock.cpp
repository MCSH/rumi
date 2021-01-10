#include "CodeBlock.h"
#include "../base.h"
#include "../LLContext.h"

void CodeBlock::prepare(CC *cc){
  cc->pushContext();

  for(Statement *s: stmts)
    s->prepare(cc);

  b = cc->popContext();
}

void CodeBlock::compile(CC *cc){
  cc->pushContext(b);

  for(Statement *s: stmts)
    s->compile(cc);

  cc->popContext();
}

void CodeBlock::codegen(CC *cc){
  cc->pushContext(b);

  for(Statement *s: stmts)
    s->codegen(cc);

  cc->popContext();
}
