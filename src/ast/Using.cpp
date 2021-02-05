#include "Using.h"
#include "../base.h"
#include "Named.h"
#include "Struct.h"
#include "MemAccess.h"
#include "PointerType.h"

Using::Using(Expression *e)
    : e(e)
{}

void Using::prepare(CC *cc){
  e->prepare(cc);
}

void Using::compile(CC *cc){
  // TODO compile me!
  e->compile(cc);

  Type *t = e->type(cc)->baseType(cc);

  if(PointerType *pt = dynamic_cast<PointerType*>(t))
    t = pt->innerType;

  if(StructType *st = dynamic_cast<StructType *>(t)){
    for(auto d: st->members){
      Named *named = new Named();
      named->id = d->id;
      named->type = d->type;
      named->alloca = 0;
      cc->registerNamed(d->id, named);
    }
  } else {
    graceFulExit(dbg, "Can't use using on " + t->toString());
  }

  // TODO using enums
}

void Using::codegen(CC *cc){
  Type *t = e->type(cc)->baseType(cc);

  if(PointerType *pt = dynamic_cast<PointerType*>(t))
    t = pt->innerType;

  if(StructType *st = dynamic_cast<StructType *>(t)){
    for(auto d: st->members){
      auto n = cc->lookup(d->id);
      MemAccess ma(e, d->id);
      ma.prepare(cc);
      ma.compile(cc);
      n->alloca = ma.allocagen(cc);
    }
  } else {
    graceFulExit(dbg, "Can't use using on " + t->toString());
  }
}
