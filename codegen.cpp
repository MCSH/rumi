#include "codegen.h"
#include "node.h"
#include "type.h"
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <string>

// TODO improve all the typeid hash_coding.
// TODO improve error handling

typedef CompileContext CC;

CC* init_context();
void codegen(Statement* stmt, CC *context);
Type* resolveType(Expression *expr, CC *cc);
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

void funcGen(FunctionDefine *fd, CC *cc){
  // TODO
  auto _ft = fd->sign;
  std::vector<llvm::Type *> args;
  // TODO args, somehow
  bool has_varargs = false;

  auto type = typeGen(fd->sign->returnT, cc);

  llvm::FunctionType *fT = llvm::FunctionType::get(type, args, has_varargs);

  auto f = llvm::Function::Create(fT, llvm::Function::ExternalLinkage, *_ft->name, cc->module.get());

  llvm::BasicBlock *bblock = llvm::BasicBlock::Create(cc->context, "entry", f);
  cc->builder->SetInsertPoint(bblock);

  // TODO handle arguments

  // TODO generate body
  for(auto s: *fd->body->stmts){
    // TODO
    codegen(s, cc);
  }

  // TODO verify returns

  llvm::verifyFunction(*f);
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
      // TODO WTF?!
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
  llvm::AllocaInst *alloc = cc->builder->CreateAlloca(t, 0, vd->name->c_str());
  cc->setVariable(vd->name, alloc, vd);

  if(vd->exp){
    // set variable to expr
    auto exp = exprGen(vd->exp, cc);
    cc->builder->CreateStore(exp, alloc);
  }
}

void variableAssignGen(VariableAssign *va, CC *cc){
  llvm::AllocaInst *alloc = cc->getVariableAlloca(va->name);
  if(!alloc){
    printf("Unknown variable %s\n", va->name->c_str());
    exit(1);
  }
  auto e = exprGen(va->exp, cc);
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

void codegen(Statement* stmt, CC *cc){
  auto t = typeid(*stmt).hash_code();

  if(t == typeid(FunctionDefine).hash_code())
    return funcGen((FunctionDefine*)stmt, cc);

  if(t == typeid(ReturnStatement).hash_code())
    return retGen((ReturnStatement*)stmt, cc);

  if(t == typeid(VariableAssign).hash_code())
    return variableAssignGen((VariableAssign*) stmt, cc);

  if(t == typeid(VariableDecl).hash_code())
    return variableDeclGen((VariableDecl *) stmt, cc);

  printf("Unknown codegen for class of type %s\n", typeid(*stmt).name());
  exit(1);
}

llvm::Value* exprGen(Expression *exp, CC *cc){
  auto t = typeid(*exp).hash_code();

  if(t == typeid(IntValue).hash_code())
    return intValueGen((IntValue*) exp, cc);
  if(t == typeid(VariableExpr).hash_code())
    return variableExprGen((VariableExpr*)exp, cc);

  printf("Unknown exprgen for class of type %s\n", typeid(*exp).name());
  exit(1);
  return NULL;
}

llvm::Type* typeGen(Type *type, CC *cc){
  auto t = typeid(*type).hash_code();

  if(t == typeid(IntType).hash_code())
    return llvm::Type::getInt64Ty(cc->context);

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

  printf("Unknown resolveType for a class of type %s\n", typeid(*expr).name());
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
  std::tie (alloca, std::ignore) = *this->getVariable(name); 
  return alloca;
}

VariableDecl *CompileContext::getVariableDecl(std::string *name){
  VariableDecl *d;
  std::tie(std::ignore, d) = *this->getVariable(name);
  return d;
}
