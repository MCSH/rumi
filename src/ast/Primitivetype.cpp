#include "PrimitiveType.h"
#include "../base.h"
#include "../LLContext.h"
#include <ostream>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>

PrimitiveType::PrimitiveType(TypeEnum key)
  : key(key)
{
}

void PrimitiveType::compile(CC *cc) {}
void PrimitiveType::prepeare(CC *cc) {}

void *PrimitiveType::typegen(CC *cc){

  switch(key){
  case t_int:
    return llvm::IntegerType::get(cc->llc->context, 64);
  case t_unit:
    return llvm::Type::getVoidTy(cc->llc->context);
  }

  cc->debug(NONE) << "Not supporting this type in primitivetypes!" << std::endl;
  exit(1);

}
