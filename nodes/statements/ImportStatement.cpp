#include "ImportStatement.h"
#include "../../Context.h"
#include <unistd.h> // chdir
#include <libgen.h> // dirname

int yyparse();
extern "C" FILE *yyin;

ImportStatement::~ImportStatement() {
  delete name;
  if (stmts)
    delete stmts;
}

void ImportStatement::codegen(CodegenContext *cc){
  if (name->compare("compiler") == 0) {
    cc->import_compiler = true;
  }

  for (auto s : *stmts)
    s->codegen(cc);
}

void ImportStatement::compile(CompileContext *cc){
  // TODO improve

  ImportStatement *is = this; // TODO because I'm lazy
  char *cwd = get_current_dir_name();
  int len = is->name->size()+1;
  int start = 0;
  const char *s = is->name->c_str();

  bool has_qt = false;
  bool hasExt = false;

  if(s[0] == '"'){
    has_qt = true;
    len-=2;
    start = 1;
  }

  if(s[len-1] == 'm' && s[len-2] == 'u' && s[len-3] == 'r' && s[len-4]=='.'){
    hasExt = true;
  } else {
    len += 4;
  }

  char *fname = (char*)malloc(sizeof(char*) *len);

  for(int i = start; i < len; i++){
    fname[i-start] = (*is->name)[i];
  }

  if(!hasExt){
    fname[len-2] = 'm';
    fname[len-3] = 'u';
    fname[len-4] = 'r';
    fname[len-5] = '.';
  }

  fname[len-1] = '\0';

  yyin = fopen(fname, "r");
  if(!yyin){
    // try the $RUMI_PATH
    char *path = getenv("RUMI_PATH");
    if(!path){
      printf("Could not import file \"%s\"\n", fname);
      printf("RUMI_PATH was not set either\n");
      exit(1);
    }

    char *newpath = (char*)malloc(strlen(path)+strlen(fname)+2);
    strcpy(newpath, path);
    // TODO memory leak
    // free(path);

    strcat(newpath, "/");
    strcat(newpath, fname);

    // printf("No local file %s, trying %s\n", fname, path);

    yyin = fopen(newpath, "r");

    if(!yyin){
      printf("Could not import file \"%s\"\n", fname);
      printf("tried %s too\n", newpath);
      exit(1);
    }
  }
  chdir(dirname(fname)); // TODO change for windows
  extern std::vector<Statement *> *mainProgramNode;
  yyparse();
  auto p = mainProgramNode;

  p = array_compile(p, cc);
  // merge me with the mainProgram
  is->stmts = p;
  chdir(cwd); // Compile will change cwd, so go back for mod generating.
}
