#include "compiler.h"
#include "codegen.h"
#include <stdio.h>
#include <unistd.h>

std::string getModuleName(char *name){
  std::string ans(name);
  return ans.append(".o");
}

int main(int argc, char **argv){
  if(argc != 2){
    printf("No file, abort\n");
    return 1;
  }
  std::string modName = getModuleName(argv[1]);
  char *cwd = get_current_dir_name();

  auto statements = compile(argv[1]);

  chdir(cwd); // Compile will change cwd, so go back for mod generating.

  codegen(statements, modName, true);

  return 0;
}
