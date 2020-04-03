#include "Context.h"

void Context::pushBlock(BlockContext *b) {
  b->parent = currentB;
  currentB = b;
}

void Context::popBlock() { currentB = currentB->parent; }

Context::Context() { currentB = &global; }

BlockContext *Context::getBlock() { return currentB; }

BlockContext *Context::getParentBlock() {
  if (currentB->parent)
    return currentB->parent;

  return &global;
}

Type *Context::getVariableType(std::string *name) {
  for (auto i = currentB; i != 0; i = i->parent) {
    auto vars = i->_vars;
    auto p = vars.find(*name);
    if (p != vars.end())
      return p->second;
  }
  return global._vars[*name];
}

StructStatement *Context::getStruct(std::string *name) {
  for (auto i = currentB; i != 0; i = i->parent) {
    auto vars = i->structs;
    auto p = vars.find(*name);
    if (p != vars.end())
      return p->second;
  }
  return global.structs[*name];
}

InterfaceStatement *Context::getInterface(std::string *name) {
  for (auto i = currentB; i != 0; i = i->parent) {
    auto vars = i->interfaces;
    auto p = vars.find(*name);
    if (p != vars.end())
      return p->second;
  }
  return global.interfaces[*name];
}

FunctionSignature *Context::getFunction(std::string *name) {
  for (auto i = currentB; i != 0; i = i->parent) {
    auto vars = i->functions;
    auto p = vars.find(*name);
    if (p != vars.end())
      return p->second;
  }
  return global.functions[*name];
}

FunctionDefine *Context::getCurrentFunction() {
  for (auto i = currentB; i != 0; i = i->parent) {
    auto f = i->currentFunction;
    if (f)
      return f;
  }
  return NULL;
}

CompileDirective *Context::getDirective(std::string *name) {
  for (auto i = currentB; i != 0; i = i->parent) {
    auto vars = i->directives;
    auto p = vars.find(*name);
    if (p != vars.end())
      return p->second;
  }
  return global.directives[*name];
}
