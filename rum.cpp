#include <cstdio>
#include <cstdlib>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include <llvm/IR/Type.h>
#include <vector>
#include <sstream>
#include "llvm/IR/Verifier.h"
#include "nodes.h"

int yyparse();
extern "C" FILE *yyin;

int main(int argc, char **argv){
  if (argc != 2){
    printf("Provide only one file\n");
    exit(1);
  }

  char *fileName = argv[1];
  yyin = fopen(fileName, "r");

  extern std::vector<Statement *> *mainProgramNode;
  yyparse();

  CC cc;

  cc.module = new llvm::Module("module", cc.context);
  cc.builder = new llvm::IRBuilder<>(cc.context);

  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  auto targetTriple = llvm::sys::getDefaultTargetTriple();

  std::string Error;
  auto target = llvm::TargetRegistry::lookupTarget(targetTriple, Error);

  if (!target) {
    llvm::errs() << Error;
    return 1;
  }

  auto CPU = "generic";
  auto Features = "";

  llvm::TargetOptions opt;
  auto RM = llvm::Optional<llvm::Reloc::Model>(
      llvm::Reloc::Model::PIC_); // Make it PIC
  auto targetMachine =
      target->createTargetMachine(targetTriple, CPU, Features, opt, RM);

  cc.module->setDataLayout(targetMachine->createDataLayout());
  cc.module->setTargetTriple(targetTriple);

  for(auto stmt: *mainProgramNode){
    stmt->compile(&cc);
  }

  cc.module->print(llvm::outs(), nullptr);
  llvm::verifyModule(*cc.module);

  cc.module->materializeAll();

  cc.module->getNamedValue("main");

  auto output = std::string(fileName)+".o";
  std::error_code EC;
  llvm::raw_fd_ostream dest(output, EC, llvm::sys::fs::F_None);

  if (EC) {
    llvm::errs() << "Could not open file: " << EC.message();
    return 1;
  }

  llvm::legacy::PassManager pass;
  //auto fileType = llvm::TargetMachine::CGFT_ObjectFile;
  auto fileType = llvm::CGFT_ObjectFile;

  if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
    llvm::errs() << "Target Machine can't emit a file of this type";
    return 1;
  }

  pass.run(*(cc.module));
  dest.flush();


  return 0;
}

void FunctionDefine::compile(CC *cc){
  auto f = this->fs->signgen(cc);
  auto fargs = this->fs->args;

  unsigned idx = 0;
  for(auto &arg: f->args()){
    if ((*fargs)[idx]->is_vararg)
      break;
    arg.setName(*(*fargs)[idx++]->name);
  }

  llvm::BasicBlock *bblock = llvm::BasicBlock::Create(cc->context, "entry", f);
  cc->builder->SetInsertPoint(bblock);
  cc->blocks.push_back(new BC(bblock));

  bool isVoid = typeid(*fs->type).hash_code() == typeid(VoidType).hash_code();

  llvm::Value *ss = nullptr;

  // handle arguments
  int i = 0;
  for(auto &arg: f->args()){
    auto a = (*fargs)[i];
    llvm::AllocaInst *alloc = cc->builder->CreateAlloca(a->type->typegen(cc), 0, *a->name);
    cc->setVariable(a->name, alloc, a);
    cc->builder->CreateStore(&arg, alloc);
    i++;
  }

  // generate Body
  for(auto s: *this->cb->stmts){
    s->compile(cc);
  }

  llvm::verifyFunction(*f);
  cc->blocks.pop_back();
}

void FunctionSignature::compile(CC *cc){
  this->signgen(cc);
}

llvm::Function *FunctionSignature::signgen(CC *cc){
  if(cc->global->funcs.find(*this->name) != cc->global->funcs.end()){
    // We are overwriting
    auto prevF = cc->module->getFunction(*this->name);
    return prevF;
  }
  cc->global->funcs[*this->name] = this;
  std::vector<llvm::Type *> args;
  bool has_varargs = false;
  for(auto arg: *this->args){
    if(arg->is_vararg){
      has_varargs = true;
      break;
    }

    args.push_back(arg->type->typegen(cc));
  }

  auto type = this->type->typegen(cc);

  llvm::FunctionType *ft = llvm::FunctionType::get(type, args, has_varargs);

  llvm::Function *f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, *this->name, *cc->module);

  return f;
}


void VariableDecl::compile(CC *cc){
  if(!type)
    type = this->expr->gettype(cc);

  llvm::Type *t = this->type->typegen(cc);

  if(!t){
    printf("Unknown type %s\n", ((StructType *)this->type)->name->c_str());
    exit(1);
  }

  auto bblock = cc->blocks.back()->bblock;
  //llvm::IRBuilder<> TmpB(bblock, bblock->getParent()->getBasicBlockList().begin()->begin());
  llvm::IRBuilder<> TmpB(&bblock->getParent()->front(), bblock->getParent()->begin()->begin());
  llvm::AllocaInst *alloc = TmpB.CreateAlloca(t, 0, this->name->c_str());
  cc->setVariable(this->name, alloc, this);

  if(this->expr){
    auto expr = this->expr->exprgen(cc);
    cc->builder->CreateStore(expr, alloc);
  }
}

void CodeBlock::compile(CC *cc){
  for(auto s: *stmts)
    s->compile(cc);
}

void StructStatement::compile(CC *cc){
  std::vector<llvm::Type *> members_t;

  auto s = llvm::StructType::create(cc->context, members_t, *this->name);
  cc->setStruct(this->name, s, this);

  for(auto m: *this->args){
    members_t.push_back(m->type->typegen(cc));
  }

  s->setBody(members_t);

}

bool is_br_ok(CC *cc){
  return 
  (cc->builder->GetInsertBlock()->empty() ||(
      cc->builder->GetInsertPoint()
              ->getPrevNonDebugInstruction()
              ->getOpcode() != llvm::Instruction::Br &&
      cc->builder->GetInsertPoint()
      ->getPrevNonDebugInstruction()
      ->getOpcode() != llvm::Instruction::Ret));
}

void WhileStatement::compile(CC *cc){
  llvm::Function *f = cc->builder->GetInsertBlock()->getParent();

  llvm::BasicBlock
    *condB = llvm::BasicBlock::Create(cc->context, "whilecond", f),
    *whileB = llvm::BasicBlock::Create(cc->context, "while"),
    *mergeB = llvm::BasicBlock::Create(cc->context, "whilecont");

  cc->builder->CreateBr(condB);

  cc->blocks.push_back(new BC(condB));
  cc->builder->SetInsertPoint(condB);

  llvm::Value *cond = this->expr->exprgen(cc);
  if(cond->getType()!=llvm::Type::getInt16Ty(cc->context)){
    cond = cc->builder->CreateIntCast(cond, llvm::Type::getInt64Ty(cc->context), true);
  }
  cond = cc->builder->CreateICmpNE(cond, llvm::ConstantInt::get(llvm::Type::getInt64Ty(cc->context), 0, false), "whilecond");
  cc->builder->CreateCondBr(cond, whileB, mergeB);
  cc->blocks.pop_back();

  // While Body
  f->getBasicBlockList().push_back(whileB);
  cc->blocks.push_back(new BC(whileB));
  cc->builder->SetInsertPoint(whileB);
  this->stmt->compile(cc);
  if(is_br_ok(cc))
    cc->builder->CreateBr(condB);
  cc->blocks.pop_back();

  
  f->getBasicBlockList().push_back(mergeB);
  cc->builder->SetInsertPoint(mergeB);
}

void IfStatement::compile(CC *cc){
  llvm::Function *f = cc->builder->GetInsertBlock()->getParent();

  llvm::BasicBlock *ifB = llvm::BasicBlock::Create(cc->context, "if", f),
    *elseB,
    *mergeB = llvm::BasicBlock::Create(cc->context, "ifcont");

  if(this->stmt2)
    elseB = llvm::BasicBlock::Create(cc->context, "else");
  else
    elseB = mergeB;

  llvm::Value *cond = this->expr->exprgen(cc);
  if(cond->getType()!=llvm::Type::getInt64Ty(cc->context)){
    cond = cc->builder->CreateIntCast(cond, llvm::Type::getInt64Ty(cc->context), true);
  }
  cond = cc->builder->CreateICmpNE(
      cond,
      llvm::ConstantInt::get(llvm::Type::getInt64Ty(cc->context), 0, false),
      "ifcond");

  cc->builder->CreateCondBr(cond, ifB, elseB);

  // If Body
  cc->blocks.push_back(new BC(ifB));
  cc->builder->SetInsertPoint(ifB);
  this->stmt1->compile(cc);
  if(is_br_ok(cc)){
    cc->builder->CreateBr(mergeB);
  }
  cc->blocks.pop_back();

  // Else
  if(this->stmt2){
    cc->blocks.push_back(new BC(elseB));
    f->getBasicBlockList().push_back(elseB);
    cc->builder->SetInsertPoint(elseB);
    this->stmt2->compile(cc);
    if (is_br_ok(cc)) {
      cc->builder->CreateBr(mergeB);
    }
    cc->blocks.pop_back();
  }

  f->getBasicBlockList().push_back(mergeB);
  cc->builder->SetInsertPoint(mergeB);
}

void VariableAssign::compile(CC *cc){
  auto alloc = this->var->getAlloca(cc);
  auto val = this->expr->exprgen(cc);
  if( alloc->getType()->getTypeID() == llvm::Type::PointerTyID // We might not even have a pointer here in structs.
     && val->getType() != alloc->getType()->getPointerElementType()){
    val = cc->builder->CreateBitOrPointerCast(val, this->var->gettype(cc)->typegen(cc));
    //val = cc->builder->CreateBitOrPointerCast(val, alloc->getType()->getPointerElementType());
    //val = cc->builder->CreateBitOrPointerCast(val, alloc->getType());
  }
  cc->builder->CreateStore(val, alloc);
}

void ReturnStatement::compile(CC *cc){
  if (this->expr){
    cc->builder->CreateRet(this->expr->exprgen(cc));
  } else {
    cc->builder->CreateRetVoid();
  }
}

llvm::Value *VariableExpr::exprgen(CC *cc){
  auto val = cc->getVariable(this->name);
  if(!val){
    printf("variable not found %d\n", lineno);
    exit(1);
  }
  return cc->builder->CreateLoad(val);
}

llvm::Value *BinaryOperation::exprgen(CC *cc){
  // TODO
  auto *lhs = this->lhs->exprgen(cc);
  auto *rhs = this->rhs->exprgen(cc);

  auto ltt = this->lhs->gettype(cc);
  auto rtt = this->rhs->gettype(cc);

  if(!ltt){
    printf("unknown type for lhs line %d\n", lineno);
    exit(1);
  }

  if(!rtt){
    printf("unknown type for rhs line %d\n", lineno);
    exit(1);
  }
  
  auto lt = typeid(*ltt).hash_code();
  auto rt = typeid(*rtt).hash_code();

  auto it = typeid(IntType).hash_code();
  auto ft = typeid(FloatType).hash_code();

  auto pt = typeid(PointerType).hash_code();


  // TODO change this based on type somehow?
  if((lt == it || lt == ft) && (rt== it || rt == ft)){
    if(lhs->getType() != rhs->getType()){
      //rhs = cc->builder->CreateBitOrPointerCast(rhs, lhs->getType());
      if(lt == rt){
        if(lt == it){
          rhs = cc->builder->CreateIntCast(rhs, lhs->getType(), ((IntType *)this->rhs->gettype(cc))->sign);
        } else {
          rhs = cc->builder->CreateFPCast(rhs, lhs->getType());
        }
      } else if(lt == it){
        rhs = cc->builder->CreateFPToSI(rhs, lhs->getType());
      } else {
        rhs = cc->builder->CreateSIToFP(rhs, lhs->getType());
      }
    }
  if (this->ops == EQ){
    return cc->builder->CreateICmpEQ(lhs, rhs);
  }
    llvm::Instruction::BinaryOps instr;

    switch (this->ops) {
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


  if (this->ops == EQ){
    if(lhs->getType() != rhs->getType()){
      if(lhs->getType()->isPointerTy())
        rhs = cc->builder->CreateIntToPtr(rhs, lhs->getType());
      else if(rhs->getType()->isPointerTy())
        lhs = cc->builder->CreateIntToPtr(lhs, rhs->getType());
    }
    return cc->builder->CreateICmpEQ(lhs, rhs);
  }

  if(((pt == lt && rt == it) || (pt == it && rt == pt)) &&
     (this->ops == PLUS || this->ops == SUB)){
    // TODO handle the type in compiler?
    // TODO handling pointer

    // Set lhs to the pointer
    if(rt == pt){
      auto tmp = lhs;
      lhs = rhs;
      rhs = tmp;

      auto tmp2 = this->lhs;
      this->lhs = this->rhs;
      this->rhs = tmp2;
    }

    // TODO cast rhs to i64 (or 32 dep on arch)

    if(this->ops == SUB){
      auto mone = llvm::ConstantInt::get(cc->context, llvm::APInt(64, -1, false));
      rhs = cc->builder->CreateBinOp(llvm::Instruction::Mul, rhs, mone);
    }

    auto t = this->lhs->gettype(cc)->typegen(cc);

    std::vector<llvm::Value *> indices(1);
    indices[0] = rhs;
    return cc->builder->CreateInBoundsGEP(t->getPointerElementType(), lhs, indices, "ptrarrptr");
  }

  printf("Can't run the expression on line %d\n", this->lhs->lineno);
  exit(1);
}

llvm::Value *PointerExpr::exprgen(CC *cc){
  this->ptr->getAlloca(cc);
}

llvm::Value *FunctionCallExpr::exprgen(CC *cc){
  llvm::Function *calleeF = cc->module->getFunction(this->name->c_str());
  if(!calleeF){
    printf("Function not found on line %d\n", lineno);
    exit(1);
  }
  bool is_void = calleeF->getReturnType()->isVoidTy();
  std::vector<llvm::Value *> argsV;
  for(auto e: *this->args){
    argsV.push_back(e->exprgen(cc));
  }

  if(is_void)
    return cc->builder->CreateCall(calleeF, argsV);
  return cc->builder->CreateCall(calleeF, argsV, "calltmp");
}

llvm::Value *IntValue::exprgen(CC *cc){
  return llvm::ConstantInt::get(llvm::Type::getInt64Ty(cc->context), this->val, true);
}

llvm::Type *IntType::typegen(CC *cc){
  switch(this->size){
  case 0:
    return llvm::Type::getInt64Ty(cc->context);
  case 1:
    return llvm::Type::getInt1Ty(cc->context);
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

llvm::Type *FloatType::typegen(CC *cc){
  switch(size){
  case 0:
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

llvm::Type *StringType::typegen(CC *cc){
  return llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(cc->context));
}

llvm::Type *PointerType::typegen(CC *cc){
  auto base = this->base->typegen(cc);
  if(!base){
    printf("Can't resolve type on line %d\n", lineno);
    exit(1);
  }
  return llvm::PointerType::getUnqual(base);
}

llvm::Value *PointerAccessExpr::exprgen(CC *cc){
  return cc->builder->CreateLoad(this->ptr->exprgen(cc), "ptra");
}

StringValue::StringValue(std::string *v){
    std::stringstream ss{""};

    for (int i = 1; i < v->length() - 1; i++) {
      if (v->at(i) == '\\') {
        switch (v->at(i + 1)) {
        case 'n':
          ss << '\n';
          i++;
          break;
        case '"':
          ss << '\"';
          i++;
          break;
        default:
          ss << '\\';
        }
      } else {
        ss << v->at(i);
      }
    }

    this->val = new std::string(ss.str());
    delete v;
}

llvm::Value *StringValue::exprgen(CC *cc){
  return cc->builder->CreateGlobalStringPtr(*this->val);
}

llvm::Type *VoidType::typegen(CC *cc){
  return llvm::Type::getVoidTy(cc->context);
}

llvm::Type *AnyType::typegen(CC *cc){
  return llvm::Type::getInt64PtrTy(cc->context);
}

llvm::Type *StructType::typegen(CC *cc){
  return cc->getStructType(this->name);
}

llvm::Value *VariableExpr::getAlloca(CC *cc){
  return cc->getVariable(this->name);
}

Type *VariableExpr::gettype(CC *cc){
  auto v = cc->getVariableDecl(this->name);
  return v->type;
}

Type *PointerExpr::gettype(CC *cc){
  return new PointerType(this->ptr->gettype(cc));
}

Type *PointerAccessExpr::gettype(CC *cc){
  return ((PointerType *)this->ptr->gettype(cc))->base;
}

llvm::Value *PointerExpr::getAlloca(CC *cc){}

Type *FunctionCallExpr::gettype(CC *cc){
  auto f = cc->getFunction(this->name);
  if(!f){
    printf("function %s not found\n", this->name->c_str());
    exit(1);
  }
  return f->type;
}

llvm::Value *MemberExpr::exprgen(CC *cc) {
  auto member_ptr = this->getAlloca(cc); // Level is handled there
  llvm::Value *loaded_member = cc->builder->CreateLoad(member_ptr, "loadtmp");
  return loaded_member;
}

llvm::Value *MemberExpr::getAlloca(CC *cc){
  int member_ind = 0;

  auto tmpe = this->base->gettype(cc);
  // TODO level?
  int level = 0;
  while(PointerType *p = dynamic_cast<PointerType *>(tmpe)){
    tmpe = p->base;
    level ++;
  }

  auto t = (StructType *) tmpe;
  auto ss = cc->getStruct(t->name);
  auto st = (llvm::StructType *) cc->getStructType(t->name);

  if(!ss){
    printf("Struct not found on line %d\n", lineno);
    exit(1);
  }

  for(auto m : *ss->args){
    if(m->name->compare(*this->name) == 0){
      break;
    }
    member_ind++;
  }

  llvm::Value *member_index = llvm::ConstantInt::get(cc->context, llvm::APInt(32, member_ind, true));

  llvm::Value *alloc;
  alloc = this->base->getAlloca(cc);

  for (int i = 0; i < level; i++) {
    alloc = cc->builder->CreateLoad(alloc, "paccess");
  }
  
  std::vector<llvm::Value *> indices(2);
  indices[0] = llvm::ConstantInt::get(cc->context, llvm::APInt(32, 0, true));
  indices[1] = member_index;

  return cc->builder->CreateInBoundsGEP(st, alloc, indices, "memberptr");
}

Type *MemberExpr::gettype(CC *cc){
  auto tmpe = this->base->gettype(cc);

  while (PointerType *p = dynamic_cast<PointerType *>(tmpe)) {
    tmpe = p->base;
  }

  auto t = (StructType *)tmpe;
  auto ss = cc->getStruct(t->name);

  // error when ss not found
  if (!ss) {
    printf("Struct %s was not found, line %d\n", t->name->c_str(),
           this->lineno);
    exit(1);
  }

  for (auto m : *ss->args) {
    if (m->name->compare(*this->name) == 0) {
      return m->type;
    }
  }
  printf("Member type not found, memberExprType on line %d\n", lineno);
  exit(1);
  return nullptr;
}

void CC::setVariable(std::string *name, llvm::AllocaInst *alloca, VariableDecl *vd){
  BC * b;
  if(this->blocks.size()){
    b = this->blocks.back();
  } else {
    b = this->global;
  }
  b->alloca[*name] = alloca;
  b->vars[*name] = vd;
}

void CC::setStruct(std::string *name, llvm::StructType *st, StructStatement *s){
  BC * b;
  if(this->blocks.size()){
    b = this->blocks.back();
  } else {
    b = this->global;
  }
  b->structs[*name] = s;
  b->stypes[*name] = st;
}

llvm::AllocaInst *CC::getVariable(std::string *name){
  for(auto bc = this->blocks.rbegin(); bc != this->blocks.rend(); bc++){
    if((*bc)->alloca.find(*name) != (*bc)->alloca.end()){
      return (*bc)->alloca[*name];
    }
  }

  return 0;
  // return global->alloca[*name];
}

VariableDecl *CC::getVariableDecl(std::string *name){
  for(auto bc = this->blocks.rbegin(); bc != this->blocks.rend(); bc++){
    if((*bc)->alloca.find(*name) != (*bc)->alloca.end()){
      return (*bc)->vars[*name];
    }
  }

  return 0;
  // return global->alloca[*name];
}

llvm::StructType *CC::getStructType(std::string *name){
  for(auto bc = this->blocks.rbegin(); bc != this->blocks.rend(); bc++){
    if((*bc)->alloca.find(*name) != (*bc)->alloca.end()){
      return (*bc)->stypes[*name];
    }
  }

  return global->stypes[*name];
}

StructStatement *CC::getStruct(std::string *name){
  for(auto bc = this->blocks.rbegin(); bc != this->blocks.rend(); bc++){
    if((*bc)->alloca.find(*name) != (*bc)->alloca.end()){
      return (*bc)->structs[*name];
    }
  }

  return global->structs[*name];
}

FunctionSignature *CC::getFunction(std::string *name){
  return global->funcs[*name];
}
