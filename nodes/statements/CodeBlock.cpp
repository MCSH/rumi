#include "CodeBlock.h"
#include "../../Context.h"

CodeBlock::~CodeBlock() {
  for (auto s : *stmts) {
    delete s;
  }

  delete stmts;
}

void CodeBlock::codegen(CodegenContext *cc){
  // TODO
  cc->defered.push_back(new std::vector<Statement*>());

  for(auto s: *this->stmts){
    s->codegen(cc);
  }

  handleDefer(cc);
}

void CodeBlock::compile(Context *cc){
  auto cb = this; // TODO lazy
  cc->blocks.push_back(new BlockContext());
  for (auto s : *cb->stmts) {
    s->compile(cc);
  }
  cc->blocks.pop_back();
}
