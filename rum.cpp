#include "compiler.h"
#include "codegen.h"
#include <stdio.h>

int main(int argc, char **argv){
  if(argc != 2){
    printf("No file, abort\n");
    return 1;
  }
  auto statements = compile(argv[1]);
  codegen(statements);

  return 0;
}
