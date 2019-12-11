#include <fstream> // TODO move to compiler.h

int yyparse();
extern "C" FILE *yyin;

void compile(char *fileName){
  yyin = fopen(fileName, "r");
  yyparse();
}
