#include "compiler.h"

int yyparse();
extern "C" FILE *yyin;

std::vector<Statement *>* compile(char *fileName){
  yyin = fopen(fileName, "r");
  extern std::vector<Statement *> *mainProgramNode;
  yyparse();

  return mainProgramNode;
}
