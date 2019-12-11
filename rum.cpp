#include "compiler.h"
#include <stdio.h>

int main(int argc, char **argv){
  if(argc == 2){
    compile(argv[1]);
  } else {
    printf("No file, abort\n");
    return 1;
  }
}
