#include "CodeBlock.h"
#include "../../Context.h"
#include "../../codegen.h"

CodeBlock::~CodeBlock() {
  for (auto s : *stmts) {
    delete s;
  }

  delete stmts;
}

void CodeBlock::codegen(Context *cc){
  // TODO
  cc->defered.push_back(new std::vector<Statement*>());

  for(auto s: *this->stmts){
    s->codegen(cc);
  }

  handleDefer(cc);
}

void CodeBlock::compile(Context *cc){
  auto cb = this; // TODO lazy
  cc->blocks.push_back((BlockContext*)this);
  for (auto s : *stmts) {
    s->compile(cc);
  }
  cc->blocks.pop_back();
}
