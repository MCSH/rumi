#include "codegen.h"
#include "node.h"
#include "type.h"
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <string>

typedef CompileContext CC;

CC* init_context();
void codegen(Statement* stmt, CC *context);
llvm::Value* exprGen(Expression *exp, CC *cc);
llvm::Type* typeGen(Type*t, CC *cc);

CC* codegen(std::vector<Statement *> *statements, std::string outfile){
  // TODO use this!
  typeid(statements).hash_code();

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
    // TODO
    printf("this shouldn't happen %s \n", ec.message().c_str());
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
  auto type = llvm::Type::getInt64Ty(cc->context);
  // type, somehow TODO
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
    auto etype = vd->exp->resolveType();
    if(vd->t && !vd->t->compatible(etype)){
      // The var type and expr type are incompatible, throw
      // TODO
    }

    if(!vd->t){
      vd->t = etype;
    } else {
      delete etype;
    }
  }

  auto t = typeGen(vd->t, cc);
  llvm::AllocaInst *alloc = cc->builder->CreateAlloca(t, 0, vd->name->c_str());
  cc->setVariable(vd->name, alloc);

  if(vd->exp){
    // set variable to expr
    auto exp = exprGen(vd->exp, cc);
    cc->builder->CreateStore(exp, alloc);
  }
}

void variableAssignGen(VariableAssign *va, CC *cc){
  // TODO
  llvm::AllocaInst *alloc = cc->getVariable(va->name);
  if(!alloc){
    // TODO user error
    printf("Unknown variable %s\n", va->name->c_str());
    exit(1);
  }
  auto e = exprGen(va->exp, cc);
  cc->builder->CreateStore(e, alloc);
}

llvm::Value* variableExprGen(VariableExpr *ve, CC *cc){
  // TODO
  llvm::AllocaInst *alloc = cc->getVariable(ve->name);
  if(!alloc){
    // TODO user error
    printf("Unknown variable %s\n", ve->name->c_str());
    exit(1);
  }

  auto load = cc->builder->CreateLoad(alloc, "readtmp");

  return load;
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

llvm::AllocaInst *CompileContext::getVariable(std::string *name){
  BlockContext *b = this->currentBlock();

  // TODO recursive and global search?

  return b->variables[*name];
}

void CompileContext::setVariable(std::string *name, llvm::AllocaInst *var){
  BlockContext *b = this->currentBlock();
  b->variables[*name] = var;
}
