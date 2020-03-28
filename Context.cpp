#include "Context.h"

void Context::pushBlock(BlockContext *b){
  b->parent = currentB;
  currentB = b;
}

void Context::popBlock(){
  currentB = currentB->parent;
}
