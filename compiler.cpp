#include "compiler.h"
#include "nodes/nodes.h"
#include <cstring>
#include <unistd.h> // chdir
#include <libgen.h> // dirname

// TODO functions and methods must be traversed once before compiling just so we have everything on record, for handling of casts and calls, etc.

int yyparse();
extern "C" FILE *yyin;

typedef CompileContext CC;

//void compile(Statement *stmts, CC* cc);
//Type *resolveType(Expression *exp, CC *cc);

CompileContext *compile(std::vector<Statement *> *stmts){
  CC *cc = new CC();
  
  cc->codes = stmts;

  for(auto s: *stmts){
    s->compile(cc);
  }

  return cc;
}

std::vector<Statement *>* array_compile(std::vector<Statement *> *stmts, CC *cc){
  for(auto s: *stmts)
    s->compile(cc);

  return stmts;
}

CompileContext* compile(char *fileName){
  yyin = fopen(fileName, "r");

  chdir(dirname(fileName)); // TODO change for windows

  extern std::vector<Statement *> *mainProgramNode;
  yyparse();
  auto p = mainProgramNode;

  return compile(p);
}


Expression *castCompile(Type *exprType, Type *baseType, Expression *e, CC *cc, Node *n, bool expl){
  // TODO, get a custom string for error reporting
  // Node n is wanted only for line no
  // Converting the third value, which is of the first type to second type.
  Compatibility c = baseType->compatible(exprType);

  if(c == ExpCast && !expl){
    printf("Can't implicity cast %s to %s\n",
           exprType->displayName().c_str(),
           baseType->displayName().c_str());
    printf("On line %d\n", n->lineno);
    exit(1);
  }

  // check for interfaces
  // resolve pointers.... augh
  Type *bt = baseType;
  int blevel = 0;
  int elevel = 0;
  Type *et = exprType;

  while(PointerType *pt = dynamic_cast<PointerType*>(bt)){
    bt = pt->base;
    blevel++;
  }

  while(PointerType *pt = dynamic_cast<PointerType*>(et)){
    et = pt->base;
    elevel++;
  }

  if(blevel + 1 == elevel)
  if(InterfaceType *it = dynamic_cast<InterfaceType*>(bt)){
    if(StructType *st = dynamic_cast<StructType*>(et)){
      // TODO change this later
      if(elevel != 1){
        printf("We don't support pointer of pointer operations between struct and interfaces, on line %d\n", n->lineno);
        exit(1);
      }
      
      // check for conversion manually
      // TODO
      InterfaceStatement *is = cc->getInterface(it->name);
      StructStatement *ss = cc->getStruct(st->name);
      if(!is){
        printf("Undefined interface %s on line %d\n", it->name->c_str(), n->lineno);
        exit(1);
      }
      if(!ss){
        printf("Undefined struct %s on line %d\n", st->name->c_str(), n->lineno);
        exit(1);
      }

      is->implements[*ss->name] = ss;

      for(auto m: *is->members){
        // check to see if struct has the same thing
        auto n = ss->methods[*m->name];
        if(!n){
          printf("Struct %s doesn't implement %s (%s) of interface %s on line %d\n",
                 st->displayName().c_str(), m->name->c_str(),
                 m->fType->displayName().c_str(),
                 it->displayName().c_str(), n->lineno);
          exit(1);
        }
        // TODO n without the first type
        FunctionType *nt = (FunctionType*)n->sign->fType->clone();
        nt->args->erase(nt->args->begin());
        if(m->fType->compatible(nt)!=OK){
          printf("Struct %s doesn't implement %s (%s) of interface %s on line %d\n",
                 st->displayName().c_str(), m->name->c_str(),
                 m->fType->displayName().c_str(),
                 it->displayName().c_str(), n->lineno);
          printf("%s implemented %s with type %s\n", st->displayName().c_str(),
                 m->name->c_str(), n->sign->fType->displayName().c_str());
          exit(1);
        }
        delete nt;
      }
      // If we reached here everything is fine, just return the expression
      // TODO we might want to pointer it later.
      return e;
    }
    // if we reached here, we can't do anything
  }

  // TODO check the other way, interface to struct

  if(c == UNCOMPATIBLE){
    printf("Uncompatible type conversion between %s and %s\n",
           exprType->displayName().c_str(),
           baseType->displayName().c_str());
    printf("On line %d\n", n->lineno);
    exit(1);
  }

  if(c == OK)
    return e;

  Expression* cast = new CastExpr(baseType, e);

  cast->exprType = baseType;

  return cast;
}

