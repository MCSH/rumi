#include "BinOpDef.h"
#include "Arg.h"
#include "Struct.h"
#include "Type.h"
#include "Named.h"
#include "../base.h"

BinOpDef::BinOpDef(std::string id, std::string op, Type *rhs, Function *f)
  : f(f)
{
  // argument count
  if(f->args.size() != 2){
    graceFulExit(dbg, "Binary operator must have exactly two arguments");
  }

  // verify argument types
  auto a1 = f->args[0]->type->toString();
  auto a2 = f->args[1]->type->toString();
  auto ea1 = id;
  auto ea2 = rhs->toString();

  if(a1 == ea1) firstPtr = false;
  else if(a1 == "*" + ea1) firstPtr = true;
  else graceFulExit(dbg, "First argument of binary operator is not of the correct type.\nIt should either be " + ea1 + " or *" + ea1 + ". Instead, it was " + a1);
  if(a2 == ea2) secondPtr = false;
  else if(a2 == "*" + ea2) secondPtr = true;
  else graceFulExit(dbg, "Second argument of binary operator is not of the correct type.\nIt should either be " + ea2 + " or *" + ea2 + ". Instead, it was " + a2);
  // store for later
  methodName = id + "."+op+"."+rhs->toString();
  structName = id;
  this->rhs = rhs;
}

void BinOpDef::prepare(CC *cc){
  // verify id is a struct
  Named *named = cc->lookup(structName);
  if(!named || !named->isType){
    graceFulExit(dbg, "Can't define binary opeators with non-type lhs");
  }
  
  StructType *st = dynamic_cast<StructType *>(named->type);
  if(!st){
    graceFulExit(dbg, "Can't defint binary operators on non-structs");
  }

  f->id = methodName;

  st->addBinOp(cc, this);

  f->prepare(cc);
}

void BinOpDef::compile(CC *cc){
  f->compile(cc);
}

void BinOpDef::codegen(CC *cc){
  f->codegen(cc);
}
