#include "CompileStatement.h"
#include "../../Context.h"
#include "../types/StructType.h"
#include "FunctionDefine.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include <cstdio>

void exitCallback(void *c, int status) { exit(status); }

void import_compiler(llvm::ExecutionEngine *EE, CodegenContext *cc) {
  // Setup the compiler object

  // == compiler$exit := (c: compiler, status: int)-> void ==
  {
    // Get the current function and create a replacement
    llvm::Function *f = EE->FindFunctionNamed("compiler$exit");
    llvm::Function *n = llvm::Function::Create(
        f->getFunctionType(), llvm::Function::ExternalLinkage,
        "compiler$exit_replace", *cc->module);

    // Creaete a basic block
    auto bb = llvm::BasicBlock::Create(cc->context, "entry", n);
    llvm::IRBuilder<> builder(bb, bb->begin());
    builder.SetInsertPoint(bb);

    // Cast the callback function to int, and then the int to function pointer
    // inside llvm
    auto fp = builder.CreateIntToPtr(
        llvm::ConstantInt::get(llvm::Type::getInt64Ty(cc->context),
                               (uint64_t)&exitCallback, false),
        f->getFunctionType()->getPointerTo());

    // Set up the args
    std::vector<llvm::Value *> *args;
    args = new std::vector<llvm::Value *>();

    auto inargs = n->args();

    // Set the first argument to the first incoming arguments, compiler
    // args->push_back(inargs.begin());

    for (auto &arg : inargs)
      args->push_back(&arg);

    /*
      // nullptr, removed in favor of inargs[0]
    args->push_back(builder.CreateIntToPtr(
        llvm::ConstantInt::get(llvm::Type::getInt64Ty(cc->context), 0, true),
        llvm::Type::getInt64Ty(cc->context)->getPointerTo()));
    */
    builder.CreateCall(fp, *args);
    builder.CreateRetVoid();

    f->replaceAllUsesWith(n);
    n->takeName(f);
    f->eraseFromParent();
  }
}

CompileStatement::~CompileStatement() {
  delete name;
  delete s;
}
void CompileStatement::codegen(CodegenContext *cc) {
  // TODO
  if (this->name->compare("compile") == 0) {
    // Compile the main statement
    // Ensure it's a function
    FunctionDefine *df;
    if (!(df = dynamic_cast<FunctionDefine *>(this->s))) {
      printf("@compile directive must be used with a function, line %d.\n",
             this->lineno);
    }

    // TODO ensure return type of int
    llvm::Function *f = df->funcgen(cc);

    auto args = df->sign->args;

    llvm::ExecutionEngine *EE =
        llvm::EngineBuilder(std::unique_ptr<llvm::Module>(cc->module)).create();

    // Set up compiler objects
    if (cc->import_compiler)
      import_compiler(EE, cc);

    bool ran = false;

    int retval = 0;

    if (args->size() == 1) {
      if (PointerType *pt = dynamic_cast<PointerType *>((*args)[0]->t)) {
        if (StructType *st = dynamic_cast<StructType *>(pt->base)) {
          if (st->name->compare("compiler") == 0) {
            // Set it to compler.
            int (*mainf)(void *) =
                (int (*)(void *))EE->getFunctionAddress(*df->sign->name);
            retval = mainf(0);
            ran = true;
          } else
            printf("arg 1's struct should be named compiler\n");
        } else
          printf("arg 1 should be a pointer to struct\n");
      } else
        printf("arg 1 should be a pointer to struct\n");
    } else if (args->size() == 0) {
      int (*mainf)() = (int (*)())EE->getFunctionAddress(*df->sign->name);
      retval = mainf();
      ran = true;
    }

    if (!ran) {
      printf(
          "Incorrect number of arguments for compie directive %s on line %d\n",
          this->name->c_str(), this->lineno);
      exit(1);
    }

    if (retval) {
      printf("Compile directive %s exited with value %d on line %d\n",
             this->name->c_str(), retval, this->lineno);
      if (retval != 1) {
        printf("Aborting\n");
        exit(1);
      }
    }

    /*
    llvm::GenericValue gv = EE->runFunction(f, noargs);

    int retval = gv.IntVal.getLimitedValue();
    */

    EE->removeModule(cc->module);

  } else {
    printf("Unknown compile directive %s on line %d\naborting\n",
           this->name->c_str(), this->lineno);
    exit(1);
  }
}
void CompileStatement::compile(Context *cc) {
  s->compile(cc);

  // Now set it to be run
  cc->compiles.push_back(this);
}
