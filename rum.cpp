#include "compiler.h"
#include "codegen.h"
#include <stdio.h>

std::string getModuleName(char *name){
  std::string ans(name);
  return ans.append(".o");
}

int main(int argc, char **argv){
  if(argc != 2){
    printf("No file, abort\n");
    return 1;
  }
  auto statements = compile(argv[1]);
  codegen(statements, getModuleName(argv[1]), true);

  return 0;
}
