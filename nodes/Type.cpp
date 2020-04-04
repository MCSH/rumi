#include "Type.h"
#include "types/AnyType.h"
#include "types/FloatType.h"
#include "types/IntType.h"
#include "types/StringType.h"
#include "types/VoidType.h"

#define Case(x,t) if(name->compare(x)==0)return new t;

Type *resolve_type(std::string *name){
  Case("int", IntType());
  Case("f32", FloatType(32));
  Case("f64", FloatType(64));
  Case("string", StringType());
  Case("any", AnyType());
  Case("void", VoidType());
  Case("u8", IntType(8,0));
  Case("u16", IntType(16,0));
  Case("u32", IntType(32,0));
  Case("u64", IntType(64,0));
  Case("s8", IntType(8,1));
  Case("s16", IntType(16,1));
  Case("s32", IntType(32,1));
  Case("s64", IntType(64,1));
  Case("bool", IntType(1));

  // TODO handle arrays in resolve type
}
