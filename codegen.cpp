#include "codegen.h"
#include "node.h"
#include "type.h"
#include <bits/stdint-uintn.h>
#include <cstdio>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <string>

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/GenericValue.h"
// TODO improve all the typeid hash_coding.
// TODO improve error handling
// TODO check types on function call
// TODO check types on function return
// TODO codegen should not throw any errors.

// TODO current problem with interface methods is improper casting
// TODO OR it is not handling global vptrs

typedef CodegenContext CC;

CC* init_context();
void codegen(Statement* stmt, CC *context);
llvm::Value *getAlloca(Expression *expr, CC *cc);
llvm::Value* exprGen(Expression *exp, CC *cc);
llvm::Type* typeGen(Type*t, CC *cc);
llvm::Value *castGen(Type *exprType, Type *baseType, llvm::Value *e, CC *cc, Node *n, bool expl, llvm::AllocaInst *alloc=nullptr);

CC* codegen(std::vector<Statement *> *statements, std::string outfile, bool print, bool ofile){
  CC *context = init_context();

  CC *cc = context; // Because I'm lazy, fix TODO

  for(auto stmt: *statements){
    codegen(stmt, context);
  }

  // create the vtables
  for(auto stmt: *statements){
    if(InterfaceStatement *is = dynamic_cast<InterfaceStatement*>(stmt)){

      for(auto tmp : is->implements){
        StructStatement *ss = tmp.second;

        std::string vptr_name = "$_" + *ss->name + "$" + *is->name;
        auto vtype = cc->getInterfaceVtableType(is->name);
        llvm::Value *vptr = cc->module->getOrInsertGlobal(
            vptr_name, vtype);

        auto vtable = cc->module->getNamedGlobal(vptr_name);

        std::vector<llvm::Constant*> v;

        for (int i = 0; i < is->members->size(); i++) {
          auto m = (*is->members)[i];

          llvm::Function *f =
              cc->module->getFunction((*ss->name) + "$" + (*m->name));

          auto c = llvm::ConstantExpr::getBitCast(f, llvm::Type::getInt64PtrTy(cc->context));

          v.push_back(c);

          // Set it in the vptr?
        }

        vtable->setInitializer(
            llvm::ConstantStruct::getAnon(cc->context, v, false));
      }
    }
  }

  // ostream, assebly annotation writer, preserve order, is for debug
  if(print)
    context->module->print(llvm::outs(), nullptr);

  llvm::verifyModule(*context->module);

  if(ofile){
    std::error_code ec;
    llvm::raw_fd_ostream buffer(outfile, ec);

    if (ec) {
      printf("Error openning object file: %s \n", ec.message().c_str());
    }

    auto module = context->module;
    llvm::WriteBitcodeToFile(*module, buffer);
  }

  return context;
}


CC* init_context(){
  CC *cc = new CC();
  // cc->module = llvm::make_unique<llvm::Module>("my cool jit", cc->context);
  cc->module = new llvm::Module("my cool jit", cc->context);
  cc->builder = new llvm::IRBuilder<>(cc->context);
  return cc;
}

llvm::Function* funcSignGen(FunctionSignature *fs, CC *cc){
  // TODO
  auto fargs = fs->args;
  std::vector<llvm::Type *> args;
  bool has_varargs = false;
  for(auto arg: *fargs){
    if(arg->vardiac){
      has_varargs = arg->vardiac; // Only the last one matters.
      break;
    }
    args.push_back(typeGen(arg->t, cc));
  }
  // TODO vargs, somehow

  auto type = typeGen(fs->returnT, cc);

  llvm::FunctionType *fT = llvm::FunctionType::get(type, args, has_varargs);

  llvm::Function* f;

  if(fs->isLocal){
    // TODO choose the best linkage
    f = llvm::Function::Create(fT, llvm::Function::PrivateLinkage, *fs->name, *cc->module);
  } else {
    f = llvm::Function::Create(fT, llvm::Function::ExternalLinkage, *fs->name, *cc->module);
  }

  return f;
}

void handleDefer(CC *cc){
  // TODO fix
  auto defered = cc->defered.back();
  for(auto i = defered->rbegin(); i != defered->rend(); i++){
    codegen(*i, cc);
  }
  // TODO memory leak
  defered->clear();
  delete defered;
  cc->defered.pop_back();
}

llvm::Function* funcGen(FunctionDefine *fd, CC *cc){
  // TODO

  cc->defered.push_back(new std::vector<Statement*>());

  auto f = funcSignGen(fd->sign, cc);
  auto fargs = fd->sign->args;

  unsigned idx = 0;
  for(auto &arg: f->args()){
    if((*fargs)[idx]->vardiac)
      break;
    arg.setName(*(*fargs)[idx++]->name);
  }

  auto preIp = cc->builder->GetInsertBlock();

  llvm::BasicBlock *bblock = llvm::BasicBlock::Create(cc->context, "entry", f);
  cc->builder->SetInsertPoint(bblock);
  cc->block.push_back(new CodegenBlockContext(bblock));

  llvm::BasicBlock *endblock = llvm::BasicBlock::Create(cc->context, "end", f);
  auto cbc = cc->block.back();
  cbc->endblock = endblock;

  bool isVoid = typeid(*fd->sign->returnT).hash_code() == typeid(VoidType).hash_code();

  llvm::AllocaInst *ra;

  // TODO check for void
  if(!isVoid){
    ra  = cc->builder->CreateAlloca(typeGen(fd->sign->returnT, cc), 0,
                                        "returnval");
    cbc->returnAlloca = ra;
  }

  // stackrestore at end, before returns
  llvm::Value *ss = nullptr;
  if(fd->dynamicStack){
    ss = cc->builder->CreateCall(llvm::Intrinsic::getDeclaration(
        cc->module, llvm::Intrinsic::stacksave));
    std::vector<llvm::Value *> a;
    a.push_back(ss);
    cc->builder->SetInsertPoint(endblock);
    cc->builder->CreateCall(llvm::Intrinsic::getDeclaration(cc->module, llvm::Intrinsic::stackrestore), a);
  }
  cc->builder->SetInsertPoint(bblock);


  // handle arguments
  int i = 0;
  for(auto &arg: f->args()){
    // TODO is it redundent to convert arg to alloca and store?
    // TODO varargs
    auto a = (*fargs)[i]; // This argument
    llvm::AllocaInst *alloc = cc->builder->CreateAlloca(typeGen(a->t, cc), 0, *a->name);
    cc->setVariable(a->name, alloc, a);
    cc->builder->CreateStore(&arg, alloc);
    i++;
  }

  // generate body
  for(auto s: *fd->body->stmts){
    codegen(s, cc);
  }

  // TODO we should raise an error for return-less functions
  // cc->builder->CreateBr(endblock);
  if(cc->builder->GetInsertPoint()->getPrevNonDebugInstruction()->getOpcode() != llvm::Instruction::Br){
    cc->builder->CreateBr(endblock);
  }

  cc->builder->SetInsertPoint(endblock);

  handleDefer(cc);
  
  if(isVoid)
    cc->builder->CreateRetVoid();
  else 
    cc->builder->CreateRet(cc->builder->CreateLoad(ra));

  llvm::verifyFunction(*f);

  cc->block.pop_back();

  if(fd->sign->name->compare("main") == 0){
    cc->mainF = f;
  }

  cc->builder->SetInsertPoint(preIp);

  return f;
}

void retGen(ReturnStatement* rt, CC *cc){
  // TODO check void
  if(rt->exp){
    cc->builder->CreateStore(exprGen(rt->exp, cc), cc->getReturnAlloca());
  }
  // No special thing needed for void
  cc->builder->CreateBr(cc->getEndBlock());
}

llvm::Value* intValueGen(IntValue* i, CC *cc){
  // TODO Check max size
  int value = i->size;
  if(i->val)
    value = atoi(i->val->c_str());
  // TODO check params?
  return llvm::ConstantInt::get(llvm::Type::getInt64Ty(cc->context), value, true);
}

void variableDeclGen(VariableDecl *vd, CC *cc) {
  // TODO maybe we need the block first for allocation?
  auto t = typeGen(vd->t, cc);
  if (!t) {
    printf("Unknown type %s\n", ((StructType *)vd->t)->name->c_str());
    exit(1);
  }

  auto bblock = cc->block.back()->bblock;

  if (auto at = dynamic_cast<ArrayType *>(vd->t)) {
    if (at->exp) {
      // stacksave/stackrestore? TODO
      auto count = exprGen(at->exp, cc);
      count = castGen(at->exp->exprType, at->base, count, cc, vd, true);
      auto alloc = cc->builder->CreateAlloca(t, 0, count, vd->name->c_str());
      cc->setVariable(vd->name, alloc, vd);
      return;
    }
  }
  // Normal variable OR array of fixed size

  // TODO handle global
  llvm::IRBuilder<> TmpB(bblock, bblock->begin());

  llvm::AllocaInst *alloc = TmpB.CreateAlloca(t, 0, vd->name->c_str());
  cc->setVariable(vd->name, alloc, vd);

  // TODO if not array, handle array with exp seperately
  if (vd->exp) {
    // set variable to expr
    auto exp = exprGen(vd->exp, cc);
    cc->builder->CreateStore(exp, alloc);
  }

  if(typeid(*vd->t).hash_code() == typeid(StructType).hash_code()){
    StructStatement *ss = cc->getStructStruct(((StructType*)vd->t)->name);
    if(ss->has_initializer){
      for(int i=0; i < ss->members->size(); i++){
        auto vd = (*ss->members)[i];
        if(vd->exp){

          // We add +1 because 0 is vptr
          llvm::Value* member_index = llvm::ConstantInt::get(cc->context, llvm::APInt(32, i+1, true));

          std::vector<llvm::Value *> indices(2);
          indices[0] =
              llvm::ConstantInt::get(cc->context, llvm::APInt(32, 0, true));
          indices[1] = member_index;

          llvm::Value *member_ptr =
              cc->builder->CreateInBoundsGEP(t, alloc, indices, "memberptr");

          cc->builder->CreateStore(exprGen(vd->exp, cc), member_ptr);
        }
      }
    }
  }
}

llvm::Value *castGen(Type *exprType, Type *baseType, llvm::Value *e, CC *cc,
                     Node *n, bool expl, llvm::AllocaInst *alloc) {
  // Node n is wanted only for line no
  // Converting the third value, which is of the first type to second type.
  Compatibility c = baseType->compatible(exprType);

  // TODO most of this should be handled in castCompile and is redundent.
  
  if(c == ExpCast && !expl){
    printf("Can't implicity cast %s to %s\n",
           exprType->displayName().c_str(),
           baseType->displayName().c_str());
    printf("On line %d\n", n->lineno);
    printf("Compiler bug reported from codegen::castGen");
    exit(1);
  }


  // check for interfaces
  // resolve pointers.... augh
  Type *_bt = baseType;
  Type *_et = exprType;

  while(PointerType *pt = dynamic_cast<PointerType*>(_bt)){
    _bt = pt->base;
  }

  while(PointerType *pt = dynamic_cast<PointerType*>(_et)){
    _et = pt->base;
  }

  if(InterfaceType *it = dynamic_cast<InterfaceType*>(_bt)){
    if(StructType *st = dynamic_cast<StructType*>(_et)){
      // Create an interface
      llvm::Type * intType = cc->getInterfaceType(it->name);

      if(!alloc)
        alloc = cc->builder->CreateAlloca(intType, 0, "interface");

      // store the poinnter with proper casting

      // We assume that e is a pointer to the struct.
      std::vector<llvm::Value *> indices(2);
      indices[0] = llvm::ConstantInt::get(cc->context, llvm::APInt(32, 0, true));
      indices[1] = llvm::ConstantInt::get(cc->context, llvm::APInt(32, 1, true));

      llvm::Value *ptr = cc->builder->CreateInBoundsGEP(intType, alloc, indices, "obj");

      cc->builder->CreateStore(
          cc->builder->CreateBitCast(e, llvm::Type::getInt64PtrTy(cc->context)),
          ptr);

      // set the vptr
      std::vector<llvm::Value *> indices2(2);
      indices2[0] = llvm::ConstantInt::get(cc->context, llvm::APInt(32, 0, true));
      indices2[1] = llvm::ConstantInt::get(cc->context, llvm::APInt(32, 0, true));

      llvm::Value *vptr =
          cc->module->getOrInsertGlobal("$_" + *st->name + "$" + *it->name,
                                        cc->getInterfaceVtableType(it->name));
      llvm::Value *vptrptr = cc->builder->CreateInBoundsGEP(intType, alloc, indices2, "vptr");
      cc->builder->CreateStore(vptr, vptrptr); // TODO might cause errors

      //return cc->builder->CreateLoad(inf);
      return nullptr;
    }
  }


  if(c == UNCOMPATIBLE){
    printf("Uncompatible type conversion between %s and %s\n",
           exprType->displayName().c_str(),
           baseType->displayName().c_str());
    printf("On line %d\n", n->lineno);
    printf("Compiler bug reported from codegen::castGen");
    exit(1);
  }

  if(c == OK)
    return e;

  const auto inttype = typeid(IntType).hash_code();
  const auto floattype = typeid(FloatType).hash_code();
  const auto pointertype = typeid(PointerType).hash_code();

  auto et = typeid(*exprType).hash_code();
  auto bt = typeid(*baseType).hash_code();

  if (et == inttype && bt == inttype){
    IntType *it = (IntType *)exprType;
    return cc->builder->CreateIntCast(e, typeGen(baseType, cc), it->isSigned);
  }

  if(et == floattype && bt == floattype){
    // TODO
    return cc->builder->CreateFPCast(e, typeGen(baseType, cc));
  }

  if (et == inttype && bt == floattype) {
    IntType *it = (IntType *)exprType;
    if (it->isSigned)
      return cc->builder->CreateSIToFP(e, typeGen(baseType, cc));
    else
      return cc->builder->CreateUIToFP(e, typeGen(baseType, cc));
  }

  if (et == floattype && bt == inttype) {
    IntType *it = (IntType *)exprType;
    if (it->isSigned)
      return cc->builder->CreateFPToSI(e, typeGen(baseType, cc));
    else
      return cc->builder->CreateFPToUI(e, typeGen(baseType, cc));
  }
  cc->builder->getDoubleTy()->getPrimitiveSizeInBits();
  if(et == pointertype && bt == pointertype){
    return cc->builder->CreateBitOrPointerCast(e, typeGen(baseType, cc));
  }

  // TODO implement other conversions.

  printf("un implemented implicit cast from %s to %s\n",
         exprType->displayName().c_str(), baseType->displayName().c_str());
  printf("On line %d\n", n->lineno);
  exit(1);
}

llvm::Value* arrayToPointer(llvm::Type* t, llvm::Value * alloc, CC *cc){
  std::vector<llvm::Value *> indices(2);
  indices[0] = llvm::ConstantInt::get(cc->context, llvm::APInt(64, 0, true));
  indices[1] = llvm::ConstantInt::get(cc->context, llvm::APInt(64, 0, true));
  return cc->builder->CreateInBoundsGEP(t, alloc, indices, "arrptr");
}

void variableAssignGen(VariableAssign *va, CC *cc){
  llvm::AllocaInst *alloc = (llvm::AllocaInst *) getAlloca(va->base, cc);
  
  if(!alloc){
    if(typeid(*va->base).hash_code()==typeid(VariableExpr).hash_code()){
      printf("Unknown variable %s\nThis is a compiler bug reported from codegen::variableAssignGen", ((VariableExpr*)va->base)->name->c_str());
    } else {
      printf("I don't know what is happening, we are on variableAssigngen\n");
    }
    exit(1);
  }

  if(typeid(*va->exp->exprType).hash_code()==typeid(ArrayType).hash_code()){
    // TODO check?
    auto art = (ArrayType*) va->exp->exprType;
    // auto arrayAlloc = cc->builder->CreateLoad(getAlloca(va->exp, cc));
    auto arrayAlloc = getAlloca(va->exp, cc);
    // auto at = typeGen(art->base, cc);
    auto at = typeGen(art, cc);
    auto ptr = arrayToPointer(at, arrayAlloc, cc);
    cc->builder->CreateStore(ptr, alloc);
    return ;
  }


  auto e = exprGen(va->exp, cc);

  // Type check
  // TODO this shouldn't be here, we are checking it in compiler
  auto baseType = va->base->exprType;
  auto exprType = va->exp->exprType;
  e = castGen(exprType, baseType, e, cc, va, false, alloc);
  if(e)
    cc->builder->CreateStore(e, alloc);
}

llvm::Value* variableExprGen(VariableExpr *ve, CC *cc){
  llvm::AllocaInst *alloc = cc->getVariableAlloca(ve->name);
  if(!alloc){
    // is it a function?

    llvm::Function *f = cc->module->getFunction(*ve->name);
    if(f){
      // We have a function
      return f;
    }

    printf("Unknown variable %s\n", ve->name->c_str());
    printf("This is a compiler bug reported in codegen::variableExprGen\n");
    exit(1);
  }

  auto load = cc->builder->CreateLoad(alloc, "readtmp");

  return load;
}

llvm::Value* functionCallExprGen(FunctionCallExpr *fc, CC *cc){
  // TODO args, function resolve could be improved!
  llvm::Function *calleeF = cc->module->getFunction(fc->name->c_str());
  llvm::Value *cf;
  bool is_void;
  if (!calleeF) {
    // maybe it's a function variable
    auto vd = cc->getVariableDecl(fc->name);
    ArgDecl* ad;

    if(vd && (ad=dynamic_cast<ArgDecl*>(vd)) && typeid(*ad->t).hash_code()==typeid(FunctionType).hash_code()){
      auto cd = cc->getVariableAlloca(fc->name);
      cf = cc->builder->CreateLoad(cd);
      is_void = dynamic_cast<VoidType*>(((FunctionType*)ad->t)->returnType);
    }
    else if(vd && typeid(*vd->t).hash_code() == typeid(FunctionType).hash_code()){
      // it's a function variable
      auto cd = cc->getVariableAlloca(fc->name);
      cf = cc->builder->CreateLoad(cd);
      is_void = dynamic_cast<VoidType*>(((FunctionType*)vd->t)->returnType);
    } else {
      printf("Function not found %s\n", fc->name->c_str());
      printf("line no %d\n", fc->lineno);
      printf("This is a compiler bug, reported from "
             "codegen::functionCallExprGen\n");
      exit(1);
    }
  } else {
    is_void = calleeF->getReturnType()->isVoidTy();
    cf = calleeF;
  }

  // There is another function call in the interface method call function
  std::vector<llvm::Value *> argsV;
  for(auto e: *fc->expr){
    if(e->exprType && typeid(*e->exprType).hash_code() == typeid(ArrayType).hash_code()){
      // For Arrays, send the pointer to the first element

      ArrayType *at = (ArrayType*) e->exprType;

      auto t = typeGen(at, cc);
      auto alloc = getAlloca(e, cc);

      if(at->count){
        argsV.push_back(arrayToPointer(t, alloc, cc));
      }
      else if(at->exp){
        argsV.push_back(alloc);
      }
      else{
        argsV.push_back(cc->builder->CreateLoad(alloc));
      }
    } else {
      argsV.push_back(exprGen(e, cc));
    }
  }

  if(is_void)
    return cc->builder->CreateCall(cf, argsV);
  return cc->builder->CreateCall(cf, argsV, "calltmp");
}

llvm::Value* binaryOpExprGen(BinaryOperation *bo, CC *cc){
  auto *lhs = exprGen(bo->lhs, cc);
  auto *rhs = exprGen(bo->rhs, cc);

  auto lt = typeid(*bo->lhs->exprType).hash_code();
  auto rt = typeid(*bo->rhs->exprType).hash_code();

  auto it = typeid(IntType).hash_code();
  auto ft = typeid(FloatType).hash_code();

  auto pt = typeid(PointerType).hash_code();

  // TODO change this based on type somehow?
  if((lt == it || lt == ft) && (rt== it || rt == ft)){
    llvm::Instruction::BinaryOps instr;

    switch (bo->op) {
    case Operation::PLUS:
      instr = llvm::Instruction::Add;
      break;
    case Operation::SUB:
      instr = llvm::Instruction::Sub;
      break;
    case Operation::MULT:
      instr = llvm::Instruction::Mul;
      break;
    case Operation::DIV:
      instr = llvm::Instruction::SDiv;
      break;
    case Operation::MOD:
      instr = llvm::Instruction::SRem;
      break;
    }

    return cc->builder->CreateBinOp(instr, lhs, rhs);
  }

  if(((pt == lt && rt == it) || (pt == it && rt == pt)) &&
     (bo->op == PLUS || bo->op == SUB)){
    // TODO handle the type in compiler?
    // TODO handling pointer

    // Set lhs to the pointer
    if(rt == pt){
      auto tmp = lhs;
      lhs = rhs;
      rhs = tmp;

      auto tmp2 = bo->lhs;
      bo->lhs = bo->rhs;
      bo->rhs = tmp2;
    }

    // TODO cast rhs to i64 (or 32 dep on arch)

    if(bo->op == SUB){
      auto mone = llvm::ConstantInt::get(cc->context, llvm::APInt(64, -1, false));
      rhs = cc->builder->CreateBinOp(llvm::Instruction::Mul, rhs, mone);
    }

    auto t = typeGen(bo->lhs->exprType, cc);

    std::vector<llvm::Value *> indices(1);
    indices[0] = rhs;
    return cc->builder->CreateInBoundsGEP(t->getPointerElementType(), lhs, indices, "ptrarrptr");
  }

  printf("Can't run the expression on types %s and %s on line %d\n", bo->lhs->exprType->displayName().c_str(), bo->rhs->exprType->displayName().c_str(), bo->lineno);
}

llvm::Value* stringvalueGen(StringValue *sv, CC *cc){
  return cc->builder->CreateGlobalStringPtr(*sv->val);
}

llvm::Value* memberAlloca(MemberExpr *e, CC *cc){
  int member_ind = 0; // TODO figure this out

  // maybe it's not StructType
  auto tmpe = e->e->exprType;

  while(PointerType* p = dynamic_cast<PointerType*>(tmpe)){
    tmpe = p->base;
  }

  auto t = (StructType*) tmpe;
  auto ss = cc->getStructStruct(t->name);
  auto st = (llvm::StructType*)cc->getStructType(t->name);

  for(auto m: *ss->members){
    if(m->name->compare(*e->mem) == 0){
      break;
    }
    member_ind++;
  }

  member_ind++; // first one is vptr

  llvm::Value* member_index = llvm::ConstantInt::get(cc->context, llvm::APInt(32, member_ind, true));

  llvm::Value * alloc;

  if(typeid(*e->e->exprType).hash_code()==typeid(PointerType).hash_code()){
    // it's a pointer
    alloc = exprGen(e->e, cc);
  } else {
    alloc = getAlloca(e->e, cc);
  }


  std::vector<llvm::Value *> indices(2);
  indices[0] = llvm::ConstantInt::get(cc->context, llvm::APInt(32, 0, true));
  indices[1] = member_index;

  llvm::Value *member_ptr = cc->builder->CreateInBoundsGEP(st, alloc, indices, "memberptr");
  return member_ptr;
}

llvm::Value* arrayAlloca(ArrayExpr* ae, CC *cc){
  ArrayType *at = (ArrayType*)ae->e->exprType;

  // check for count member
  if (at->count) {
    auto t = typeGen(ae->e->exprType, cc);
    auto alloc = getAlloca(ae->e, cc);
    // TODO check alloc here and everywhere else!
    auto ind = exprGen(ae->mem, cc);

    std::vector<llvm::Value *> indices(2);
    indices[0] = llvm::ConstantInt::get(cc->context, llvm::APInt(64, 0, true));
    indices[1] = ind;
    return cc->builder->CreateInBoundsGEP(t, alloc, indices, "arrptr");
  } else if(at->exp){
    // This is basically a pointer
    auto t = typeGen(ae->e->exprType, cc);
    auto alloc = getAlloca(ae->e, cc);
    std::vector<llvm::Value *> indices(1);
    indices[0] = exprGen(ae->mem, cc);
    auto tmp = cc->builder->CreateInBoundsGEP(t, alloc, indices, "ptrarrptr");
    return tmp;
  } else {
    // I have no clue how this is working, but it does.
    auto t = typeGen(ae->e->exprType, cc);
    auto alloc = getAlloca(ae->e, cc);
    alloc = cc->builder->CreateLoad(alloc);
    std::vector<llvm::Value *> indices(1);
    indices[0] = exprGen(ae->mem, cc);
    return cc->builder->CreateInBoundsGEP(t->getPointerElementType(), alloc, indices, "ptrarrptr");
  }
}

llvm::Value* pointerAccessExprAlloca(PointerAccessExpr* expr, CC *cc){
  auto load = exprGen(expr->exp, cc);
  return load;
}

llvm::Value* memberExprGen(MemberExpr *e, CC *cc){
  // TODO
  auto member_ptr = memberAlloca(e, cc);
  llvm::Value *loaded_member = cc->builder->CreateLoad(member_ptr, "loadtmp");
  return loaded_member;
}

llvm::Value* arrayExprGen(ArrayExpr *e, CC *cc){
  auto member_ptr = arrayAlloca(e, cc);
  llvm::Value *loaded_member = cc->builder->CreateLoad(member_ptr, "loadtmp");
  return loaded_member;
}

void ifGen(IfStatement *is, CC *cc){
  llvm::Function *f = cc->builder->GetInsertBlock()->getParent();

  llvm::BasicBlock *ifB = llvm::BasicBlock::Create(cc->context, "if", f),
    *elseB,
    *mergeB = llvm::BasicBlock::Create(cc->context, "ifcont");

  if(is->e)
    elseB = llvm::BasicBlock::Create(cc->context, "else");
  else
    elseB = mergeB;

  llvm::Value *cond = exprGen(is->exp, cc);

  cond = cc->builder->CreateICmpNE(cond, llvm::ConstantInt::get(llvm::Type::getInt64Ty(cc->context), 0, false), // TODO, maybe improve?
    "ifcond");

  cc->builder->CreateCondBr(cond, ifB, elseB);

  // If Body
  cc->block.push_back(new CodegenBlockContext(ifB));
  cc->builder->SetInsertPoint(ifB);
  codegen(is->i, cc);
  if (cc->builder->GetInsertPoint()
          ->getPrevNonDebugInstruction()
          ->getOpcode() != llvm::Instruction::Br) {
    cc->builder->CreateBr(mergeB);
  }
  cc->block.pop_back();

  // Else
  if(is->e){
    cc->block.push_back(new CodegenBlockContext(elseB));
    f->getBasicBlockList().push_back(elseB);
    cc->builder->SetInsertPoint(elseB);
    codegen(is->e, cc);
    if (cc->builder->GetInsertPoint()
            ->getPrevNonDebugInstruction()
            ->getOpcode() != llvm::Instruction::Br) {
      cc->builder->CreateBr(mergeB);
    }
    cc->block.pop_back();
  }

  f->getBasicBlockList().push_back(mergeB);
  cc->builder->SetInsertPoint(mergeB);
}

void blockGen(CodeBlock *cb, CC *cc){
  // TODO
  cc->defered.push_back(new std::vector<Statement*>());

  for(auto s: *cb->stmts){
    codegen(s, cc);
  }

  handleDefer(cc);
}

void whileGen(WhileStatement *ws, CC *cc){
  // TODO

  llvm::Function *f = cc->builder->GetInsertBlock()->getParent();

  llvm::BasicBlock
    *condB = llvm::BasicBlock::Create(cc->context, "whilecond", f),
    *whileB = llvm::BasicBlock::Create(cc->context, "while"),
    *mergeB = llvm::BasicBlock::Create(cc->context, "whilecont");

  cc->builder->CreateBr(condB);

  // While cond
  cc->block.push_back(new CodegenBlockContext(condB));
  cc->builder->SetInsertPoint(condB);
  llvm::Value *cond = exprGen(ws->exp, cc);
  cond = cc->builder->CreateICmpNE(cond, llvm::ConstantInt::get(llvm::Type::getInt64Ty(cc->context), 0, false), "whilecond"); // TODO improve?
  cc->builder->CreateCondBr(cond, whileB, mergeB);
  cc->block.pop_back();

  // While Body
  f->getBasicBlockList().push_back(whileB);
  cc->block.push_back(new CodegenBlockContext(whileB));
  cc->builder->SetInsertPoint(whileB);
  codegen(ws->w, cc);
  if (cc->builder->GetInsertPoint()
          ->getPrevNonDebugInstruction()
          ->getOpcode() != llvm::Instruction::Br) {
    cc->builder->CreateBr(condB);
  }
  cc->block.pop_back();

  // Cont
  f->getBasicBlockList().push_back(mergeB);
  cc->builder->SetInsertPoint(mergeB);
}

void interfaceGen(InterfaceStatement *is, CC *cc){
  // TODO

  // Create a vtable struct
  std::vector<llvm::Type *> vptr_t;

  for(auto m: *is->members){
    vptr_t.push_back(llvm::Type::getInt64PtrTy(cc->context));
  }

  auto vt = llvm::StructType::create(cc->context, vptr_t, (*is->name)+ "$_interface_vptr");

  // TODO include pointers to subinterfaces

  // create interface object
  std::vector<llvm::Type *> intf;
  intf.push_back(vt->getPointerTo());
  intf.push_back(llvm::Type::getInt64PtrTy(cc->context));

  auto it = llvm::StructType::create(cc->context, intf, (*is->name));

  cc->setInterface(is->name, it, vt, is);
}

void structGen(StructStatement *ss, CC *cc){
  // TODO

  ss->type_counter = cc->struct_type_counter;
  cc->struct_type_counter++;

  std::vector<llvm::Type *> members_t;

  std::vector<llvm::Type *> vptr_t;
  // First element is the class number

  vptr_t.push_back(llvm::Type::getInt64Ty(cc->context));

  auto vt = llvm::StructType::create(cc->context, vptr_t);

  // create the global vptr

  std::string vptr_name = "$$_vptr$"+ *ss->name;

  cc->module->getOrInsertGlobal(vptr_name, vt);
  auto vp = cc->module->getNamedGlobal(vptr_name);
  vp->setLinkage(llvm::GlobalValue::ExternalLinkage);

  // TODO set the initializer after all methods are registered.
  std::vector<llvm::Constant *> v;
  v.push_back(llvm::ConstantInt::get(llvm::Type::getInt64Ty(cc->context),
                                     ss->type_counter, false));
  vp->setInitializer(llvm::ConstantStruct::getAnon(cc->context, v, false));

  llvm::Type *vptr = vt->getPointerTo();
  members_t.push_back(vptr);


  for(auto m: *ss->members){
    members_t.push_back(typeGen(m->t, cc));
  }

  auto s = llvm::StructType::create(cc->context, members_t, *ss->name); // TODO check params
  // TODO set the compiler context
  cc->setStruct(ss->name, s, ss);
}

void deferGen(DeferStatement* ds, CC *cc){
  cc->defered.back()->push_back(ds->s);
}

llvm::Type *interfaceType(InterfaceType *it, CC *cc){
  return cc->getInterfaceType(it->name);
}

llvm::Type *structType(StructType *st, CC *cc){
  return cc->getStructType(st->name);
}

llvm::Type *intTypeGen(IntType *it, CC *cc){
  switch(it->size){
  case 0:
    /// TODO do it based on sys arch
    return llvm::Type::getInt64Ty(cc->context);
  case 8:
    return llvm::Type::getInt8Ty(cc->context);
  case 16:
    return llvm::Type::getInt16Ty(cc->context);
  case 32:
    return llvm::Type::getInt32Ty(cc->context);
  case 64:
    return llvm::Type::getInt64Ty(cc->context);
  }

  printf("Something went wrong on intTypeGen\n");
  exit(1);
  return nullptr;
}

llvm::Type *floatTypeGen(FloatType *ft, CC *cc){
  switch(ft->size){
  case 0:
    // TODO do it based on sys arch
    return llvm::Type::getFloatTy(cc->context);
  case 32:
    return llvm::Type::getFloatTy(cc->context);
  case 64:
    return llvm::Type::getDoubleTy(cc->context);
  }

  printf("Something went wrong on floatTypeGen\n");
  exit(1);
  return nullptr;
}

void exitCallback(void*c, int status){
  exit(status);
}

void import_compiler(llvm::ExecutionEngine *EE, CC *cc){
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

    for(auto &arg: inargs)
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

void compileGen(CompileStatement *stmt, CC *cc){
  // TODO
  if(stmt->name->compare("compile")==0){
    // Compile the main statement
    // Ensure it's a function
    FunctionDefine *df;
    if(!(df=dynamic_cast<FunctionDefine*>(stmt->s))){
      printf("@compile directive must be used with a function, line %d.\n", stmt->lineno);
    }

    // TODO ensure return type of int
    llvm::Function *f = funcGen(df, cc);

    auto args = df->sign->args;

    llvm::ExecutionEngine *EE = llvm::EngineBuilder(std::unique_ptr<llvm::Module>(cc->module)).create();

    // Set up compiler objects
    if(cc->import_compiler)
      import_compiler(EE, cc);

    bool ran = false;

    int retval = 0;

    if (args->size() == 1) {
      if (PointerType *pt = dynamic_cast<PointerType *>((*args)[0]->t)) {
        if (StructType *st = dynamic_cast<StructType *>(pt->base)) {
          if (st->name->compare("compiler") == 0) {
            // Set it to compler.
            int (*mainf)(void*) = (int (*)(void*))EE->getFunctionAddress(*df->sign->name);
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

    if(!ran){
      printf("Incorrect number of arguments for compie directive %s on line %d\n",
             stmt->name->c_str(), stmt->lineno);
      exit(1);
    }

    if (retval) {
      printf("Compile directive %s exited with value %d on line %d\n",
             stmt->name->c_str(), retval, stmt->lineno);
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
    printf("Unknown compile directive %s on line %d\naborting\n", stmt->name->c_str(), stmt->lineno);
    exit(1);
  }
}

llvm::Value *interfaceMethodCall(MethodCall *mce, CC *cc){

  llvm::Value *i = getAlloca(mce->e, cc);

  // find the method index
  InterfaceType *it = (InterfaceType*)mce->e->exprType;
  InterfaceStatement *is = cc->getInterfaceStatement(it->name);

  int ind = 0;

  // We are sure that it exists because otherwise compile would raise an error
  FunctionSignature *ff;
  for(ind; ind < is->members->size(); ind++){
    auto m = (*is->members)[ind];
    if(m->name->compare(*mce->name)==0){
      // we found the match
      ff = m;
      break;
    }
  }

  // get the vtable
  llvm::Type *iv = cc->getInterfaceType(it->name);
  llvm::Type * vt = cc->getInterfaceVtableType(it->name);

  std::vector<llvm::Value *> indices(2);
  indices[0] = llvm::ConstantInt::get(cc->context, llvm::APInt(32, 0, true));
  indices[1] = llvm::ConstantInt::get(cc->context, llvm::APInt(32, 0, true));
  llvm::Value *vtable =
      cc->builder->CreateInBoundsGEP(iv, i, indices, "vtable");

  // get the method pointer
  indices[1] = llvm::ConstantInt::get(cc->context, llvm::APInt(32, ind, true));
  llvm::Value *f =
    cc->builder->CreateInBoundsGEP(vt, cc->builder->CreateLoad(vtable), indices, "fptr");

  // I think we should cast f

  // change the function type when calling it
  FunctionType *ft = (FunctionType*)ff->fType->clone();

  // when calling the function, pass the pointer not the whole object
  
  ft->args->insert(ft->args->begin(), new PointerType(new IntType(64)));

  f = cc->builder->CreateBitCast(f, typeGen(ft, cc)->getPointerTo());

  // Replace the mce->expr[0] with the pointer to the struct

  indices[1] = llvm::ConstantInt::get(cc->context, llvm::APInt(32, 1, true));
  llvm::Value *sptr =
      cc->builder->CreateInBoundsGEP(iv, i, indices, "sptr");

  // call the method

  f = cc->builder->CreateLoad(f);


  std::vector<llvm::Value *> argsV;
  argsV.push_back(cc->builder->CreateLoad(sptr));

  mce->expr->erase(mce->expr->begin());
  
  for(auto e: *mce->expr){
    if(e->exprType && typeid(*e->exprType).hash_code() == typeid(ArrayType).hash_code()){
      // For Arrays, send the pointer to the first element

      ArrayType *at = (ArrayType*) e->exprType;

      auto t = typeGen(at, cc);
      auto alloc = getAlloca(e, cc);

      if(at->count){
        argsV.push_back(arrayToPointer(t, alloc, cc));
      }
      else if(at->exp){
        argsV.push_back(alloc);
      }
      else{
        argsV.push_back(cc->builder->CreateLoad(alloc));
      }
    } else {
      argsV.push_back(exprGen(e, cc));
    }
  }

  bool is_void = dynamic_cast<VoidType*>(ff->returnT);
  if(is_void){
    cc->builder->CreateCall(f, argsV);
    return 0;
  }
  return cc->builder->CreateCall(f, argsV, "calltmp");
}

void codegen(Statement* stmt, CC *cc){
  auto t = typeid(*stmt).hash_code();

  if(t == typeid(ImportStatement).hash_code()){
    ImportStatement *is = (ImportStatement*) stmt;
    if(is->name->compare("compiler") == 0){
      cc->import_compiler = true;
    }

    for(auto s : *is->stmts)
      codegen(s, cc);
    return;
  }

  if(t == typeid(FunctionDefine).hash_code()){
    funcGen((FunctionDefine*)stmt, cc);
    return;
  }

  if(t == typeid(FunctionSignature).hash_code()){
    funcSignGen((FunctionSignature*)stmt, cc);
    return;
  }

  if(t == typeid(FunctionCallExpr).hash_code()){
    functionCallExprGen((FunctionCallExpr*) stmt, cc);
    return;
  }

  if(t == typeid(ReturnStatement).hash_code())
    return retGen((ReturnStatement*)stmt, cc);

  if(t == typeid(VariableAssign).hash_code())
    return variableAssignGen((VariableAssign*) stmt, cc);

  if(t == typeid(VariableDecl).hash_code())
    return variableDeclGen((VariableDecl *) stmt, cc);

  if(t == typeid(IfStatement).hash_code())
    return ifGen((IfStatement*) stmt, cc);

  if(t == typeid(CodeBlock).hash_code())
    return blockGen((CodeBlock*) stmt, cc);

  if(t == typeid(WhileStatement).hash_code())
    return whileGen((WhileStatement*) stmt, cc);

  if(t == typeid(StructStatement).hash_code())
    return structGen((StructStatement *) stmt, cc);

  if(t == typeid(InterfaceStatement).hash_code())
    return interfaceGen((InterfaceStatement*)stmt, cc);

  if(t == typeid(DeferStatement).hash_code())
    return deferGen((DeferStatement*) stmt, cc);

  if(t == typeid(CompileStatement).hash_code())
    return compileGen((CompileStatement*)stmt, cc);

  if(t == typeid(MemberStatement).hash_code())
    return codegen(((MemberStatement*)stmt)->f, cc);

  if(t == typeid(MethodCall).hash_code()){
    MethodCall *mce = (MethodCall*) stmt;
    if(mce->f) 
      return codegen(((MethodCall*)stmt)->fce, cc);
    interfaceMethodCall(mce, cc);
    return;
  }

  printf("Unknown codegen for class of type %s\n", typeid(*stmt).name());
  exit(1);
}

llvm::Value* exprGen(Expression *exp, CC *cc){
  auto t = typeid(*exp).hash_code();

  if(t == typeid(IntValue).hash_code())
    return intValueGen((IntValue*) exp, cc);
  if(t == typeid(VariableExpr).hash_code())
    return variableExprGen((VariableExpr*)exp, cc);
  if(t == typeid(FunctionCallExpr).hash_code())
    return functionCallExprGen((FunctionCallExpr*) exp, cc);
  if(t == typeid(BinaryOperation).hash_code())
    return binaryOpExprGen((BinaryOperation *) exp, cc);
  if(t == typeid(StringValue).hash_code())
    return stringvalueGen((StringValue *) exp, cc);
  if(t == typeid(MemberExpr).hash_code())
    return memberExprGen((MemberExpr *) exp, cc);
  if(t == typeid(ArrayExpr).hash_code())
    return arrayExprGen((ArrayExpr *) exp, cc);
  if(t == typeid(CastExpr).hash_code()){
    CastExpr * ce = (CastExpr*)exp;
    auto targetType = ce->exp->exprType;
    auto baseType = ce->t;
    return castGen(targetType, baseType, exprGen(ce->exp, cc), cc, ce, true);
  }
  if(t == typeid(PointerExpr).hash_code()){
    PointerExpr *pe = (PointerExpr*) exp;
    return getAlloca(pe->exp, cc);
  }

  if(t == typeid(PointerAccessExpr).hash_code()){
    auto pae = (PointerAccessExpr*) exp;
    auto load = cc->builder->CreateLoad(exprGen(pae->exp, cc), "ptra");
    return load;
  }

  if(t == typeid(SizeofExpr).hash_code()){
    auto se = (SizeofExpr*) exp;
    auto dl = cc->module->getDataLayout();
    auto size = dl.getTypeAllocSize(typeGen(se->t, cc)); 
    // TODO memory leak
    return exprGen(new IntValue(size), cc);
  }

  if(t == typeid(MethodCall).hash_code()){
    MethodCall *mce = (MethodCall*)exp;
    if(mce->f)
      return exprGen(((MethodCall*)exp)->fce, cc);
    return interfaceMethodCall(mce, cc);
  }

  printf("Unknown exprgen for class of type %s\n", typeid(*exp).name());
  exit(1);
  return NULL;
}

llvm::Type* typeGen(Type *type, CC *cc){
  auto t = typeid(*type).hash_code();

  if(t == typeid(IntType).hash_code())
    return intTypeGen((IntType*)type, cc);

  // TODO, improve?
  if(t == typeid(StringType).hash_code())
    return llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(cc->context));

  if(t == typeid(AnyType).hash_code())
    return llvm::Type::getVoidTy(cc->context);

  if(t == typeid(VoidType).hash_code())
    return llvm::Type::getVoidTy(cc->context);

  if(t == typeid(StructType).hash_code())
    return structType((StructType*)type, cc);

  if(t == typeid(InterfaceType).hash_code())
    return interfaceType((InterfaceType*)type, cc);

  if(t == typeid(FloatType).hash_code())
    return floatTypeGen((FloatType*) type, cc);

  if(t == typeid(PointerType).hash_code()){
    PointerType *pt = (PointerType*)type;
    return llvm::PointerType::getUnqual(typeGen(pt->base, cc));
  }

  if(t == typeid(ArrayType).hash_code()){
    ArrayType *at = (ArrayType*)type;

    // check for count
    if(at->count)
      return llvm::ArrayType::get(typeGen(at->base, cc), at->count);
    else if(at->exp)
      return typeGen(at->base, cc);
    else
      return llvm::PointerType::getUnqual(typeGen(at->base, cc));
  }

  if(t == typeid(FunctionType).hash_code()){
    FunctionType *ft = (FunctionType*) type;
    std::vector<llvm::Type *> params;

    for(auto arg: *ft->args)
      params.push_back(typeGen(arg, cc));

    // TODO this should be a poitner by default

    return llvm::PointerType::getUnqual(llvm::FunctionType::get(typeGen(ft->returnType, cc), params,false)); // TODO varargs
  }

  printf("Unknown typeGen for a class of type %s\n", typeid(*type).name());
  exit(1);
  return NULL;
}

llvm::Value *getAlloca(Expression *expr, CC *cc){
  auto t = typeid(*expr).hash_code();

  if(t == typeid(VariableExpr).hash_code())
    return cc->getVariableAlloca(((VariableExpr*) expr)->name);

  if(t == typeid(MemberExpr).hash_code())
    return memberAlloca((MemberExpr *)expr, cc);

  if(t == typeid(ArrayExpr).hash_code())
    return arrayAlloca((ArrayExpr*)expr, cc);

  if(t == typeid(PointerAccessExpr).hash_code())
    return pointerAccessExprAlloca((PointerAccessExpr*) expr, cc);
  
  printf("Unknown getAlloca for a class of type %s\n", typeid(*expr).name());
  exit(1);
  return NULL;
}

std::tuple<llvm::AllocaInst *, VariableDecl *> *
CodegenContext::getVariable(std::string *name) {
  for (auto i = block.rbegin(); i != block.rend(); i++) {
    auto vars = (*i)->variables;
    auto p = vars.find(*name);
    if (p != vars.end())
      return p->second;
  }

  return global.variables[*name];
}

void CodegenContext::setVariable(std::string *name, llvm::AllocaInst *var,
                                 VariableDecl *vd) {
  CodegenBlockContext *b = this->currentBlock();
  b->variables[*name] = new std::tuple<llvm::AllocaInst*, VariableDecl*>(var, vd);
}

llvm::AllocaInst *CodegenContext::getVariableAlloca(std::string *name) {
  llvm::AllocaInst *alloca;
  auto tup = this->getVariable(name); 
  if(!tup)
    return nullptr;
  std::tie (alloca, std::ignore) = *tup;
  return alloca;
}

VariableDecl *CodegenContext::getVariableDecl(std::string *name) {
  VariableDecl *d;
  auto tup = this->getVariable(name);
  if(!tup)
    return nullptr;
  std::tie(std::ignore, d) = *tup;
  return d;
}

std::tuple<llvm::Type *, StructStatement *> *
CodegenContext::getStruct(std::string *name) {
  for (auto i = block.rbegin(); i != block.rend(); i++) {
    auto vars = (*i)->structs;
    auto p = vars.find(*name);
    if (p != vars.end())
      return p->second;
  }

  return global.structs[*name];
}

void CodegenContext::setStruct(std::string *name, llvm::Type *t,
                               StructStatement *st) {
  CodegenBlockContext *b = this->currentBlock();
  b->structs[*name] = new std::tuple<llvm::Type*, StructStatement*>(t, st);
}

llvm::Type *CodegenContext::getStructType(std::string *name) {
  llvm::Type *t;
  auto tup = this->getStruct(name); 
  if(!tup)
    return nullptr;
  std::tie (t, std::ignore) = *tup;
  return t;
}

StructStatement *CodegenContext::getStructStruct(std::string *name) {
  StructStatement *s;
  auto tup = this->getStruct(name);
  if(!tup)
    return nullptr;
  std::tie(std::ignore, s) = *tup;
  return s;
}

std::tuple<std::tuple<llvm::Type *, llvm::Type *> *, InterfaceStatement *> *
CodegenContext::getInterface(std::string *name){
  for (auto i = block.rbegin(); i != block.rend(); i++) {
    auto vars = (*i)->interfaces;
    auto p = vars.find(*name);
    if (p != vars.end())
      return p->second;
  }

  return global.interfaces[*name];
}

llvm::Type* CodegenContext::getInterfaceType(std::string *name){
  llvm::Type *t;
  auto tup = this->getInterface(name); 
  if(!tup)
    return nullptr;
  std::tuple<llvm::Type*, llvm::Type*> *tmp;
  std::tie(tmp, std::ignore) = *tup;
  std::tie(t, std::ignore) = *tmp;
  return t;
}

llvm::Type* CodegenContext::getInterfaceVtableType(std::string *name){
  llvm::Type *t;
  auto tup = this->getInterface(name); 
  if(!tup)
    return nullptr;
  std::tuple<llvm::Type*, llvm::Type*> *tmp;
  std::tie(tmp, std::ignore) = *tup;
  std::tie(std::ignore, t) = *tmp;
  return t;
}

InterfaceStatement *CodegenContext::getInterfaceStatement(std::string *name) {
  InterfaceStatement *s;
  auto tup = this->getInterface(name);
  if(!tup)
    return nullptr;
  std::tie(std::ignore, s) = *tup;
  return s;
}

void CodegenContext::setInterface(std::string *name, llvm::Type *t, llvm::Type *t2,
                               InterfaceStatement *st) {
  CodegenBlockContext *b = this->currentBlock();
  b->interfaces[*name] = new std::tuple<std::tuple<llvm::Type*, llvm::Type*>*, InterfaceStatement*>(new std::tuple<llvm::Type*, llvm::Type*>(t, t2), st);
}

llvm::BasicBlock *CodegenContext::getEndBlock(){
  for (auto i = block.rbegin(); i != block.rend(); i++) {
    auto eb = (*i)->endblock;
    if(eb)
      return eb;
  }

  return nullptr;
}

llvm::AllocaInst *CodegenContext::getReturnAlloca(){
  for (auto i = block.rbegin(); i != block.rend(); i++) {
    auto ra = (*i)->returnAlloca;
    if(ra)
      return ra;
  }

  return nullptr;
}
