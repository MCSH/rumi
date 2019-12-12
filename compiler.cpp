#include <fstream> // TODO move to compiler.h
#include "node.h"

int yyparse();
extern "C" FILE *yyin;

void compile(char *fileName){
  yyin = fopen(fileName, "r");
  extern std::vector<Statement *> *mainProgramNode;
  yyparse();
}
