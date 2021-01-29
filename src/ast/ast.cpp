#include "ast.h"
#include <iostream>
#include "../DebugInfo.h"

AST::~AST(){}

void AST::setDBG(DebugInfo *d){
  this->dbg = d;
}

void graceFulExit(DebugInfo *d, std::string error){
  if(error.size() > 0) error += " ";
  std::cerr << "Error " << error << "at " << *d << std::endl;
  exit(1);
}
