#include "codegen.h"
#include "node.h"
#include "type.h"
#include <cstdio>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <string>

// TODO improve all the typeid hash_coding.
// TODO improve error handling
// TODO check types on function call
// TODO check types on function return

// TODO cast expression type is deleted somewhere!!!
// TODO shift reduce

typedef CompileContext CC;

CC* init_context();
void codegen(Statement* stmt, CC *context);
Type* resolveType(Expression *expr, CC *cc);
llvm::Value *getAlloca(Expression *expr, CC *cc);
llvm::Value* exprGen(Expression *exp, CC *cc);
llvm::Type* typeGen(Type*t, CC *cc);

CC* codegen(std::vector<Statement *> *statements, std::string outfile){
  CC *context = init_context();

  for(auto stmt: *statements){
    codegen(stmt, context);
  }

  // ostream, assebly annotation writer, preserve order, is for debug
  context->module->print(llvm::outs(), nullptr);

  llvm::verifyModule(*context->module.get());

  std::error_code ec;
  llvm::raw_fd_ostream buffer(outfile, ec);

  if(ec){
    printf("Error openning object file: %s \n", ec.message().c_str());
  }

  auto module = context->module.get();

  llvm::WriteBitcodeToFile(*module, buffer);

  return context;
}


CC* init_context(){
  CC *cc = new CC();
  cc->module = llvm::make_unique<llvm::Module>("my cool jit", cc->context);
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

  auto f = llvm::Function::Create(fT, llvm::Function::ExternalLinkage, *fs->name, cc->module.get());

  return f;
}

void funcGen(FunctionDefine *fd, CC *cc){
  // TODO

  auto f = funcSignGen(fd->sign, cc);
  auto fargs = fd->sign->args;

  unsigned idx = 0;
  for(auto &arg: f->args()){
    if((*fargs)[idx]->vardiac)
      break;
    arg.setName(*(*fargs)[idx++]->name);
  }

  llvm::BasicBlock *bblock = llvm::BasicBlock::Create(cc->context, "entry", f);
  cc->builder->SetInsertPoint(bblock);
  cc->block.push_back(new BlockContext(bblock));

  // TODO handle arguments
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

  // TODO generate body
  for(auto s: *fd->body->stmts){
    // TODO
    codegen(s, cc);
  }

  // TODO verify returns

  llvm::verifyFunction(*f);

  cc->block.pop_back();
}

void retGen(ReturnStatement* rt, CC *cc){
  // TODO check void
  if(rt->exp)
    cc->builder->CreateRet(exprGen(rt->exp, cc));
  else
    cc->builder->CreateRetVoid();
}

llvm::Value* intValueGen(IntValue* i, CC *cc){
  // TODO Check max size
  int value = atoi(i->val->c_str());
  // TODO check params?
  return llvm::ConstantInt::get(llvm::Type::getInt64Ty(cc->context), value, true);
}

void variableDeclGen(VariableDecl *vd, CC *cc){
  // TODO maybe we need the block first for allocation?

  if(vd->exp){
    // set the type
    auto etype = resolveType(vd->exp, cc);
    if(vd->t && !vd->t->compatible(etype)){
      // The var type and expr type are incompatible, throw
      printf("first type: %s, second type %s\n", typeid(*vd->t).name(), typeid(*etype).name());
      printf("Incompatible variable type with assigned variable\n");
      exit(1);
    }

    if(!vd->t){
      vd->t = etype;
    } else {
      delete etype;
    }
  }

  auto t = typeGen(vd->t, cc);
  if(!t){
    printf("Unknown type %s\n", ((StructType*)vd->t)->name->c_str());
    exit(1);
  }
  llvm::AllocaInst *alloc = cc->builder->CreateAlloca(t, 0, vd->name->c_str());
  cc->setVariable(vd->name, alloc, vd);

  if(vd->exp){
    // set variable to expr
    auto exp = exprGen(vd->exp, cc);
    cc->builder->CreateStore(exp, alloc);
  }
}

llvm::Value *castGen(Type *exprType, Type *baseType, llvm::Value *e, CC *cc, Node *n, bool expl){
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

  if(c == UNCOMPATIBLE){
    printf("Uncompatible type conversion between %s and %s\n",
           exprType->displayName().c_str(),
           baseType->displayName().c_str());
    printf("On line %d\n", n->lineno);
    exit(1);
  }

  if(c == OK)
    return e;

  const auto inttype = typeid(IntType).hash_code();
  const auto floattype = typeid(FloatType).hash_code();

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

  // TODO implement other conversions.

  printf("un implemented implicit from cast %s to %s\n",
         exprType->displayName().c_str(), baseType->displayName().c_str());
  printf("On line %d\n", n->lineno);
  exit(1);
}

void variableAssignGen(VariableAssign *va, CC *cc){
  // llvm::AllocaInst *alloc = cc->getVariableAlloca(va->name);

  llvm::AllocaInst *alloc = (llvm::AllocaInst *) getAlloca(va->base, cc);
  
  if(!alloc){
    if(typeid(*va->base).hash_code()==typeid(VariableExpr).hash_code()){
      printf("Unknown variable %s\n", ((VariableExpr*)va->base)->name->c_str());
    } else {
      printf("I don't know what is happening, we are on variableAssigngen\n");
    }
    exit(1);
  }
  auto e = exprGen(va->exp, cc);

  // Type check
  auto baseType = resolveType(va->base, cc);
  auto exprType = resolveType(va->exp, cc);
  e = castGen(exprType, baseType, e, cc, va, false);
  cc->builder->CreateStore(e, alloc);
}

llvm::Value* variableExprGen(VariableExpr *ve, CC *cc){
  llvm::AllocaInst *alloc = cc->getVariableAlloca(ve->name);
  if(!alloc){
    printf("Unknown variable %s\n", ve->name->c_str());
    exit(1);
  }

  auto load = cc->builder->CreateLoad(alloc, "readtmp");

  return load;
}

Type* variableExprType(VariableExpr *ve, CC *cc){
  return cc->getVariableDecl(ve->name)->t->clone();
}

llvm::Value* functionCallExprGen(FunctionCallExpr *fc, CC *cc){
  // TODO args, function resolve could be improved!
  llvm::Function *calleeF = cc->module->getFunction(fc->name->c_str());
  if (!calleeF) {
    printf("Function not found %s\n", fc->name->c_str());
    exit(1);
  }

  std::vector<llvm::Value *> argsV;
  for(auto e: *fc->expr){
    argsV.push_back(exprGen(e, cc));
  }

  return cc->builder->CreateCall(calleeF, argsV, "calltmp");
}

Type* memberExprType(MemberExpr *me, CC *cc){
  auto t = (StructType*) resolveType(me->e, cc);
  auto ss = cc->getStructStruct(t->name);

  for(auto m: *ss->members){
    if(m->name->compare(*me->mem) == 0){
      return m->t;
    }
  }

  printf("Member type not found, memberExprType on line %d\n", me->lineno);
  exit(1);
}

llvm::Value* binaryOpExprGen(BinaryOperation *bo, CC *cc){
  auto *lhs = exprGen(bo->lhs, cc);
  auto *rhs = exprGen(bo->rhs, cc);

  // TODO change this based on type somehow?
  llvm::Instruction::BinaryOps instr;

  switch(bo->op){
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

Type* binaryOpExprType(BinaryOperation *bo, CC * cc){
  // TODO compare the types
  return resolveType(bo->lhs, cc);
}

llvm::Value* stringvalueGen(StringValue *sv, CC *cc){
  return cc->builder->CreateGlobalStringPtr(*sv->val);
}

llvm::Value* memberAlloca(MemberExpr *e, CC *cc){
  int member_ind = 0; // TODO figure this out

  auto t = (StructType*) resolveType(e->e, cc);
  auto ss = cc->getStructStruct(t->name);
  auto st = (llvm::StructType*)cc->getStructType(t->name);

  for(auto m: *ss->members){
    if(m->name->compare(*e->mem) == 0){
      break;
    }
    member_ind++;
  }

  llvm::Value* member_index = llvm::ConstantInt::get(cc->context, llvm::APInt(32, member_ind, true));

  auto alloc = getAlloca(e->e, cc);

  std::vector<llvm::Value *> indices(2);
  indices[0] = llvm::ConstantInt::get(cc->context, llvm::APInt(32, 0, true));
  indices[1] = member_index;

  llvm::Value *member_ptr = cc->builder->CreateInBoundsGEP(st, alloc, indices, "memberptr");
  return member_ptr;
}

llvm::Value* memberExprGen(MemberExpr *e, CC *cc){
  // TODO
  auto member_ptr = memberAlloca(e, cc);
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
  cc->block.push_back(new BlockContext(ifB));
  cc->builder->SetInsertPoint(ifB);
  codegen(is->i, cc);
  cc->builder->CreateBr(mergeB);
  cc->block.pop_back();

  // Else
  if(is->e){
    cc->block.push_back(new BlockContext(elseB));
    f->getBasicBlockList().push_back(elseB);
    cc->builder->SetInsertPoint(elseB);
    codegen(is->e, cc);
    cc->builder->CreateBr(mergeB);
    cc->block.pop_back();
  }

  f->getBasicBlockList().push_back(mergeB);
  cc->builder->SetInsertPoint(mergeB);
}

void blockGen(CodeBlock *cb, CC *cc){
  // TODO
  for(auto s: *cb->stmts){
    codegen(s, cc);
  }
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
  cc->block.push_back(new BlockContext(condB));
  cc->builder->SetInsertPoint(condB);
  llvm::Value *cond = exprGen(ws->exp, cc);
  cond = cc->builder->CreateICmpNE(cond, llvm::ConstantInt::get(llvm::Type::getInt64Ty(cc->context), 0, false), "whilecond"); // TODO improve?
  cc->builder->CreateCondBr(cond, whileB, mergeB);
  cc->block.pop_back();

  // While Body
  f->getBasicBlockList().push_back(whileB);
  cc->block.push_back(new BlockContext(whileB));
  cc->builder->SetInsertPoint(whileB);
  codegen(ws->w, cc);
  cc->builder->CreateBr(condB);
  cc->block.pop_back();

  // Cont
  f->getBasicBlockList().push_back(mergeB);
  cc->builder->SetInsertPoint(mergeB);
}

void structGen(StructStatement *ss, CC *cc){
  // TODO
  std::vector<llvm::Type *> members_t;
  for(auto m: *ss->members){
    members_t.push_back(typeGen(m->t, cc));
  }

  auto s = llvm::StructType::create(cc->context, members_t, *ss->name); // TODO check params
  // TODO set the compiler context
  cc->setStruct(ss->name, s, ss);
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

Type *castExprType(CastExpr *ce, CC *cc){
  // TODO
  return ce->t;
}


void codegen(Statement* stmt, CC *cc){
  auto t = typeid(*stmt).hash_code();

  if(t == typeid(FunctionDefine).hash_code())
    return funcGen((FunctionDefine*)stmt, cc);

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
  if(t == typeid(CastExpr).hash_code()){
    CastExpr * ce = (CastExpr*)exp;
    auto targetType = resolveType(ce->exp, cc);
    auto baseType = ce->t;
    return castGen(targetType, baseType, exprGen(ce->exp, cc), cc, ce, true);
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

  if(t == typeid(StructType).hash_code())
    return structType((StructType*)type, cc);

  if(t == typeid(FloatType).hash_code())
    return floatTypeGen((FloatType*) type, cc);

  printf("Unknown typeGen for a class of type %s\n", typeid(*type).name());
  exit(1);
  return NULL;
}

Type* resolveType(Expression *expr, CC *cc){
  auto t = typeid(*expr).hash_code();

  if(t == typeid(IntValue).hash_code())
    return new IntType();

  if(t == typeid(VariableExpr).hash_code())
    return variableExprType((VariableExpr *) expr, cc);

  if(t == typeid(BinaryOperation).hash_code())
    return binaryOpExprType((BinaryOperation *) expr, cc);

  if(t == typeid(MemberExpr).hash_code())
    return memberExprType((MemberExpr*) expr, cc);

  if(t == typeid(CastExpr).hash_code())
    return castExprType((CastExpr*) expr, cc);

  printf("Unknown resolveType for a class of type %s\n", typeid(*expr).name());
  exit(1);
  return NULL;
}

llvm::Value *getAlloca(Expression *expr, CC *cc){
  auto t = typeid(*expr).hash_code();

  if(t == typeid(VariableExpr).hash_code())
    return cc->getVariableAlloca(((VariableExpr*) expr)->name);

  if(t == typeid(MemberExpr).hash_code())
    return memberAlloca((MemberExpr *)expr, cc);
  
  printf("Unknown getAlloca for a class of type %s\n", typeid(*expr).name());
  exit(1);
  return NULL;
}

std::tuple<llvm::AllocaInst *, VariableDecl *> *CompileContext::getVariable(std::string *name){
  for (auto i = block.rbegin(); i != block.rend(); i++) {
    auto vars = (*i)->variables;
    auto p = vars.find(*name);
    if (p != vars.end())
      return p->second;
  }

  return global.variables[*name];
}

void CompileContext::setVariable(std::string *name, llvm::AllocaInst *var, VariableDecl *vd){
  BlockContext *b = this->currentBlock();
  b->variables[*name] = new std::tuple<llvm::AllocaInst*, VariableDecl*>(var, vd);
}

llvm::AllocaInst *CompileContext::getVariableAlloca(std::string *name){
  llvm::AllocaInst *alloca;
  auto tup = this->getVariable(name); 
  if(!tup)
    return nullptr;
  std::tie (alloca, std::ignore) = *tup;
  return alloca;
}

VariableDecl *CompileContext::getVariableDecl(std::string *name){
  VariableDecl *d;
  auto tup = this->getVariable(name);
  if(!tup)
    return nullptr;
  std::tie(std::ignore, d) = *tup;
  return d;
}

std::tuple<llvm::Type *, StructStatement *> *CompileContext::getStruct(std::string *name){
  for (auto i = block.rbegin(); i != block.rend(); i++) {
    auto vars = (*i)->structs;
    auto p = vars.find(*name);
    if (p != vars.end())
      return p->second;
  }

  return global.structs[*name];
}

void CompileContext::setStruct(std::string *name, llvm::Type *t, StructStatement *st){
  BlockContext *b = this->currentBlock();
  b->structs[*name] = new std::tuple<llvm::Type*, StructStatement*>(t, st);
}

llvm::Type *CompileContext::getStructType(std::string *name){
  llvm::Type *t;
  auto tup = this->getStruct(name); 
  if(!tup)
    return nullptr;
  std::tie (t, std::ignore) = *tup;
  return t;
}

StructStatement *CompileContext::getStructStruct(std::string *name){
  StructStatement *s;
  auto tup = this->getStruct(name);
  if(!tup)
    return nullptr;
  std::tie(std::ignore, s) = *tup;
  return s;
}
