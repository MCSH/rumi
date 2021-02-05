#include "FCall.h"
#include "../base.h"
#include "../LLContext.h"
#include "Cast.h"
#include "FunctionType.h"
#include "Named.h"
#include "Type.h"
#include "ast.h"

// NOTE: There are cases where prepare/compile won't be called on fcall, but we will ensure the function exists and that th arguments have their compile/prepeare called and they are type checked. E.x., struct opgen

void FCall::argError(FunctionType *ft, CC *cc){
  std::string myargStrings = "(";
  bool first = true;
  for(auto e: args){
    if(first) myargStrings += e->type(cc)->toString();
    else myargStrings += ", " + e->type(cc)->toString();
    first = false;
  }
  myargStrings += ")";
  graceFulExit(dbg, id + " doesn't conform with the definition, the required type is " + ft->toString() + " but was called with " + myargStrings);
}

void FCall::compile(CC *cc){
  // ensure function exists
  auto named = cc->lookup(id);
  if(!named){
    graceFulExit(dbg, "Couldn't find function " + id);
  }
  FunctionType *ft = dynamic_cast<FunctionType *>(named->type);
  if(!ft){
    graceFulExit(dbg, id + " isn't a function");
  }
  for(auto e: args) e->compile(cc);

  
  for(int i = 0; i < ft->args.size(); i++){
    if(i == args.size()){
      if(ft->vararg && i == ft->args.size()-1) break;
      argError(ft, cc);
    }
    auto p = ft->args[i];
    auto e = args[i];
    Compatibility c = p->compatible(cc, e->type(cc)->baseType(cc));
    if(c > ImpCast){
      cc->debug(NONE) << "Incompatible types " << e->type(cc)->toString() << " and " << p->toString() << std::endl;
      argError(ft, cc);
    } else if(c == ImpCast){
      args[i] = new Cast(e, p);
    }
  }
  if(args.size() > ft->args.size()){
    if(!ft->vararg){
      // TODO provide better error message?
      argError(ft, cc);
    }

    int s = ft->args.size();
    auto p = ft->args[s-1];
    for(int i = s; i < args.size(); i++){
      // cast to the vararg type
      auto e = args[i];
      Compatibility c = p->compatible(cc, e->type(cc)->baseType(cc));
      if (c > ImpCast) {
        cc->debug(NONE) << "Incompatible types " << e->type(cc)->toString() << " and " << p->toString() << std::endl;
        argError(ft, cc);
      } else if (c == ImpCast) {
        args[i] = new Cast(e, p);
      }
    }
  }
}

void FCall::prepare(CC *cc){
  for(auto e: args) e->prepare(cc);
}

Type *FCall::type(CC *cc){
  Named *n = cc->lookup(id);
  FunctionType *ft = dynamic_cast<FunctionType *>(n->type);
  return ft->returnType;
}

void *FCall::exprgen(CC *cc){
  llvm::Function *f = (llvm::Function*)cc->lookup(id)->alloca;
  std::vector<llvm::Value *> params;
  for(auto e: args){
    params.push_back((llvm::Value *)e->exprgen(cc));
  }
  return cc->llc->builder->CreateCall(f->getFunctionType(), f, params);
}

void *FCall::allocagen(CC *cc){
  graceFulExit(dbg, "Calling alloca on unsupported type");
  return 0;
}

void FCallStmt::compile(CC *cc){
  fc.compile(cc);
}
void FCallStmt::prepare(CC *cc){
  fc.dbg = dbg;
  fc.prepare(cc);
}
void FCallStmt::codegen(CC *cc){
  fc.exprgen(cc);
}
