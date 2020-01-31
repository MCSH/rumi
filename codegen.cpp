#include "codegen.h"
#include "nodes/types/InterfaceType.h"
#include "nodes/types/StructType.h"
#include "nodes/types/InterfaceType.h"
#include "nodes/types/IntType.h"
#include "nodes/types/FloatType.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/Verifier.h"
// TODO improve all the typeid hash_coding.
// TODO improve error handling
// TODO check types on function call
// TODO check types on function return
// TODO codegen should not throw any errors.

typedef CodegenContext CC;

CC *init_context();

CC *codegen(std::vector<Statement *> *statements, std::string outfile,
            bool print, bool ofile) {
  CC *context = init_context();

  CC *cc = context; // Because I'm lazy, fix TODO

  for (auto stmt : *statements) {
    stmt->codegen(context);
  }

  // create the vtables
  for (auto stmt : *statements) {
    if (InterfaceStatement *is = dynamic_cast<InterfaceStatement *>(stmt)) {

      for (auto tmp : is->implements) {
        StructStatement *ss = tmp.second;

        std::string vptr_name = "$_" + *ss->name + "$" + *is->name;
        auto vtype = cc->getInterfaceVtableType(is->name);
        llvm::Value *vptr = cc->module->getOrInsertGlobal(vptr_name, vtype);

        auto vtable = cc->module->getNamedGlobal(vptr_name);

        std::vector<llvm::Constant *> v;

        for (int i = 0; i < is->members->size(); i++) {
          auto m = (*is->members)[i];

          llvm::Function *f =
              cc->module->getFunction((*ss->name) + "$" + (*m->name));

          auto c = llvm::ConstantExpr::getBitCast(
              f, llvm::Type::getInt64PtrTy(cc->context));

          v.push_back(c);

          // Set it in the vptr?
        }

        vtable->setInitializer(
            llvm::ConstantStruct::getAnon(cc->context, v, false));
      }
    }
  }

  // ostream, assebly annotation writer, preserve order, is for debug
  if (print)
    context->module->print(llvm::outs(), nullptr);

  llvm::verifyModule(*context->module);

  if (ofile) {
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

CC *init_context() {
  CC *cc = new CC();
  // cc->module = llvm::make_unique<llvm::Module>("my cool jit", cc->context);
  cc->module = new llvm::Module("my cool jit", cc->context);
  cc->builder = new llvm::IRBuilder<>(cc->context);
  return cc;
}

void handleDefer(CC *cc) {
  // TODO fix
  auto defered = cc->defered.back();
  for (auto i = defered->rbegin(); i != defered->rend(); i++) {
    (*i)->codegen(cc);
  }
  // TODO memory leak
  defered->clear();
  delete defered;
  cc->defered.pop_back();
}

llvm::Value *castGen(Type *exprType, Type *baseType, llvm::Value *e, CC *cc,
                     Node *n, bool expl, llvm::AllocaInst *alloc) {
  // Node n is wanted only for line no
  // Converting the third value, which is of the first type to second type.
  Compatibility c = baseType->compatible(exprType);

  // TODO most of this should be handled in castCompile and is redundent.

  if (c == ExpCast && !expl) {
    printf("Can't implicity cast %s to %s\n", exprType->displayName().c_str(),
           baseType->displayName().c_str());
    printf("On line %d\n", n->lineno);
    printf("Compiler bug reported from codegen::castGen");
    exit(1);
  }

  // check for interfaces
  // resolve pointers.... augh
  Type *_bt = baseType;
  Type *_et = exprType;

  while (PointerType *pt = dynamic_cast<PointerType *>(_bt)) {
    _bt = pt->base;
  }

  while (PointerType *pt = dynamic_cast<PointerType *>(_et)) {
    _et = pt->base;
  }

  if (InterfaceType *it = dynamic_cast<InterfaceType *>(_bt)) {
    if (StructType *st = dynamic_cast<StructType *>(_et)) {
      // Create an interface
      llvm::Type *intType = cc->getInterfaceType(it->name);

      if (!alloc)
        alloc = cc->builder->CreateAlloca(intType, 0, "interface");

      // store the poinnter with proper casting

      // We assume that e is a pointer to the struct.
      std::vector<llvm::Value *> indices(2);
      indices[0] =
          llvm::ConstantInt::get(cc->context, llvm::APInt(32, 0, true));
      indices[1] =
          llvm::ConstantInt::get(cc->context, llvm::APInt(32, 1, true));

      llvm::Value *ptr =
          cc->builder->CreateInBoundsGEP(intType, alloc, indices, "obj");

      cc->builder->CreateStore(
          cc->builder->CreateBitCast(e, llvm::Type::getInt64PtrTy(cc->context)),
          ptr);

      // set the vptr
      std::vector<llvm::Value *> indices2(2);
      indices2[0] =
          llvm::ConstantInt::get(cc->context, llvm::APInt(32, 0, true));
      indices2[1] =
          llvm::ConstantInt::get(cc->context, llvm::APInt(32, 0, true));

      llvm::Value *vptr =
          cc->module->getOrInsertGlobal("$_" + *st->name + "$" + *it->name,
                                        cc->getInterfaceVtableType(it->name));
      llvm::Value *vptrptr =
          cc->builder->CreateInBoundsGEP(intType, alloc, indices2, "vptr");
      cc->builder->CreateStore(vptr, vptrptr); // TODO might cause errors

      // return cc->builder->CreateLoad(inf);
      return nullptr;
    }
  }

  if (c == UNCOMPATIBLE) {
    printf("Uncompatible type conversion between %s and %s\n",
           exprType->displayName().c_str(), baseType->displayName().c_str());
    printf("On line %d\n", n->lineno);
    printf("Compiler bug reported from codegen::castGen");
    exit(1);
  }

  if (c == OK)
    return e;

  const auto inttype = typeid(IntType).hash_code();
  const auto floattype = typeid(FloatType).hash_code();
  const auto pointertype = typeid(PointerType).hash_code();

  auto et = typeid(*exprType).hash_code();
  auto bt = typeid(*baseType).hash_code();

  if (et == inttype && bt == inttype) {
    IntType *it = (IntType *)exprType;
    return cc->builder->CreateIntCast(e, baseType->typeGen(cc), it->isSigned);
  }

  if (et == floattype && bt == floattype) {
    // TODO
    return cc->builder->CreateFPCast(e, baseType->typeGen(cc));
  }

  if (et == inttype && bt == floattype) {
    IntType *it = (IntType *)exprType;
    if (it->isSigned)
      return cc->builder->CreateSIToFP(e, baseType->typeGen(cc));
    else
      return cc->builder->CreateUIToFP(e, baseType->typeGen(cc));
  }

  if (et == floattype && bt == inttype) {
    IntType *it = (IntType *)exprType;
    if (it->isSigned)
      return cc->builder->CreateFPToSI(e, baseType->typeGen(cc));
    else
      return cc->builder->CreateFPToUI(e, baseType->typeGen(cc));
  }
  cc->builder->getDoubleTy()->getPrimitiveSizeInBits();
  if (et == pointertype && bt == pointertype) {
    return cc->builder->CreateBitOrPointerCast(e, baseType->typeGen(cc));
  }

  // TODO implement other conversions.

  printf("un implemented implicit cast from %s to %s\n",
         exprType->displayName().c_str(), baseType->displayName().c_str());
  printf("On line %d\n", n->lineno);
  exit(1);
}

llvm::Value *arrayToPointer(llvm::Type *t, llvm::Value *alloc, CC *cc) {
  std::vector<llvm::Value *> indices(2);
  indices[0] = llvm::ConstantInt::get(cc->context, llvm::APInt(64, 0, true));
  indices[1] = llvm::ConstantInt::get(cc->context, llvm::APInt(64, 0, true));
  return cc->builder->CreateInBoundsGEP(t, alloc, indices, "arrptr");
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
  b->variables[*name] =
      new std::tuple<llvm::AllocaInst *, VariableDecl *>(var, vd);
}

llvm::AllocaInst *CodegenContext::getVariableAlloca(std::string *name) {
  llvm::AllocaInst *alloca;
  auto tup = this->getVariable(name);
  if (!tup)
    return nullptr;
  std::tie(alloca, std::ignore) = *tup;
  return alloca;
}

VariableDecl *CodegenContext::getVariableDecl(std::string *name) {
  VariableDecl *d;
  auto tup = this->getVariable(name);
  if (!tup)
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
  b->structs[*name] = new std::tuple<llvm::Type *, StructStatement *>(t, st);
}

llvm::Type *CodegenContext::getStructType(std::string *name) {
  llvm::Type *t;
  auto tup = this->getStruct(name);
  if (!tup)
    return nullptr;
  std::tie(t, std::ignore) = *tup;
  return t;
}

StructStatement *CodegenContext::getStructStruct(std::string *name) {
  StructStatement *s;
  auto tup = this->getStruct(name);
  if (!tup)
    return nullptr;
  std::tie(std::ignore, s) = *tup;
  return s;
}

std::tuple<std::tuple<llvm::Type *, llvm::Type *> *, InterfaceStatement *> *
CodegenContext::getInterface(std::string *name) {
  for (auto i = block.rbegin(); i != block.rend(); i++) {
    auto vars = (*i)->interfaces;
    auto p = vars.find(*name);
    if (p != vars.end())
      return p->second;
  }

  return global.interfaces[*name];
}

llvm::Type *CodegenContext::getInterfaceType(std::string *name) {
  llvm::Type *t;
  auto tup = this->getInterface(name);
  if (!tup)
    return nullptr;
  std::tuple<llvm::Type *, llvm::Type *> *tmp;
  std::tie(tmp, std::ignore) = *tup;
  std::tie(t, std::ignore) = *tmp;
  return t;
}

llvm::Type *CodegenContext::getInterfaceVtableType(std::string *name) {
  llvm::Type *t;
  auto tup = this->getInterface(name);
  if (!tup)
    return nullptr;
  std::tuple<llvm::Type *, llvm::Type *> *tmp;
  std::tie(tmp, std::ignore) = *tup;
  std::tie(std::ignore, t) = *tmp;
  return t;
}

InterfaceStatement *CodegenContext::getInterfaceStatement(std::string *name) {
  InterfaceStatement *s;
  auto tup = this->getInterface(name);
  if (!tup)
    return nullptr;
  std::tie(std::ignore, s) = *tup;
  return s;
}

void CodegenContext::setInterface(std::string *name, llvm::Type *t,
                                  llvm::Type *t2, InterfaceStatement *st) {
  CodegenBlockContext *b = this->currentBlock();
  b->interfaces[*name] =
      new std::tuple<std::tuple<llvm::Type *, llvm::Type *> *,
                     InterfaceStatement *>(
          new std::tuple<llvm::Type *, llvm::Type *>(t, t2), st);
}

llvm::BasicBlock *CodegenContext::getEndBlock() {
  for (auto i = block.rbegin(); i != block.rend(); i++) {
    auto eb = (*i)->endblock;
    if (eb)
      return eb;
  }

  return nullptr;
}

llvm::AllocaInst *CodegenContext::getReturnAlloca() {
  for (auto i = block.rbegin(); i != block.rend(); i++) {
    auto ra = (*i)->returnAlloca;
    if (ra)
      return ra;
  }

  return nullptr;
}
