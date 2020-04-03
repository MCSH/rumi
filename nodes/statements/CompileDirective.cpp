#include "CompileDirective.h"
#include "../../Context.h"
#include "../types/StructType.h"
#include "../types/PointerType.h"
#include "CompileStatement.h"
#include <llvm/ExecutionEngine/ExecutionEngine.h>

// TODO compile the directive only once and store it in this->f

void CompileDirective::run(Context *cc, Statement *stmt) {
  // TODO
  // TODO check statement?

  auto args = df->sign->args;

  llvm::ExecutionEngine *EE =
      llvm::EngineBuilder(std::unique_ptr<llvm::Module>(cc->module)).create();

  // Set up compiler objects
  /* TODO do I need this?*/
  if (cc->import_compiler)
    import_compiler(EE, cc);

  bool ran = false;

  int retval = 0;
  if (args->size() == 1) {
    if (PointerType *pt = dynamic_cast<PointerType *>((*args)[0]->t)) {
      if (StructType *st = dynamic_cast<StructType *>(pt->base)) {
        if (st->name->compare("statement") == 0) {
          // Set it to statement.
          int (*mainf)(void *) =
              (int (*)(void *))EE->getFunctionAddress(*df->sign->name);
          retval = mainf(stmt);
          ran = true;
        } else {
          printf("arg 1's struct should be named statement\n");
        }
      } else {
        printf("arg 1 should be a pointer to struct\n");
      }
    } else {
      printf("arg 1 should be a pointer to struct\n");
    }
  }
  if (args->size() == 2) {
    if (PointerType *pt = dynamic_cast<PointerType *>((*args)[0]->t)) {
      if (StructType *st = dynamic_cast<StructType *>(pt->base)) {
        if (st->name->compare("compiler") == 0) {
          if (PointerType *pt = dynamic_cast<PointerType *>((*args)[1]->t)) {
            if (StructType *st = dynamic_cast<StructType *>(pt->base)) {
              if (st->name->compare("statement") == 0) {
                // Set it to statement.
                int (*mainf)(void *, void *) = (int (*)(
                    void *, void *))EE->getFunctionAddress(*df->sign->name);
                retval = mainf(0, stmt);
                ran = true;
              } else {
                printf("arg 2's struct should be named statement\n");
              }
            } else {
              printf("arg 2 should be a pointer to struct\n");
            }
          } else {
            printf("arg 2 should be a pointer to struct\n");
          }
        } else
          printf("arg 1's struct should be named compiler\n");
      } else
        printf("arg 1 should be a pointer to struct\n");
    } else
      printf("arg 1 should be a pointer to struct\n");
  } else if (args->size() == 1) {
    int (*mainf)() = (int (*)())EE->getFunctionAddress(*df->sign->name);
    retval = mainf();
    ran = true;
  }

  if (!ran) {
    printf("Incorrect number of arguments for compie directive %s on line %d\n",
           this->df->sign->name->c_str(), this->lineno);
    exit(1);
  }

  if (retval) {
    printf("Compile directive %s exited with value %d on line %d\n",
           this->df->sign->name->c_str(), retval, this->lineno);
    if (retval != 1) {
      printf("Aborting\n");
      exit(1);
    }
  }

  stmt->codegen(cc);

  EE->removeModule(cc->module);
}

CompileDirective::CompileDirective(llvm::Function *f, FunctionDefine *df)
    : f(f), df(df) {
  lineno = df->lineno;
}

CompileDirective::~CompileDirective() {
  delete f;
  delete df;
}
