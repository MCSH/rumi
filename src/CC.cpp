#include "LLContext.h"
#include "ast/ast.h"
#include "base.h"
#include "Source.h"
#include <fstream>
#include <iostream>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/IRBuilder.h>
#include "parser/DynamicParseRule.h"

// Used in outputing to nowhere.
class NullStream : public std::ostream {
public:
  NullStream() : std::ostream(nullptr) {}
  NullStream(const NullStream &) : std::ostream(nullptr) {}
};
template <class T>
const NullStream &operator<<(NullStream &&os, const T &value) {
  return os;
}

CompileContext::CompileContext(int argc, char **argv){
  this->verbosity = 0;
  for (int i = 1; i < argc; i++) {
    std::string arg = std::string(argv[i]);
    if (arg == "-v") {
      this->verbosity++;
    } else if(arg == "-o"){
      // TODO make sure this is safe
      this->outFile = std::string(argv[i+1]);
      i++;
    } else {
      this->sources.push_back(new Source(arg));
      // std::cout << argv[i] << std::endl;
    }
  }
  this->debug(Verbosity::LOW)
    << "Verbosity set at " << this->verbosity << std::endl;
  /// TODO PrintInfo and exit on arg parsing error
  parser.init(this);
}


std::ostream &CompileContext::debug(int v) {
  if (v <= this->verbosity) {
    return std::cout;
  }
  static NullStream ns;
  return ns;
}

void CompileContext::load(Source *s) {
  s->loadBuff();
  s->fetch();
  this->debug(LOW) << s->str << std::endl;
}

void CompileContext::pushContext(){
  auto b = new BlockContext();
  b -> parent = block;
  block = b;
}

void CompileContext::pushContext(BlockContext *b){
  block = b;
}

BlockContext *CompileContext::popContext(){
  auto b = block;
  block = b->parent;
  return b;
}

void CompileContext::registerNamed(std::string id, Named *n){
  /// TODO check for overriding instances;
  block->named[id] = n;

  debug(HIGH) << "Registering " << id << std::endl;
}

Named *CompileContext::lookup(std::string id){
  auto b = block;
  while(b){
    auto p = b->named.find(id);
    if(p != b->named.end()){
      return p->second;
    }

    b = b->parent;
  }

  return 0;
}

std::string CompileContext::pathResolve(std::string path){
  if(path.find(".rum") == std::string::npos){
    path += ".rum";
  }
  return path;
}
// ========== Callbacks ==========
#define STRTOCSTR(s) (char *) s.c_str()
#define CSTRTOSTR(s) std::string(s)


void exitCallback(CC *cc, int i){
  exit(i);
}

void setCallback(CC *cc, char *ckey, void *value){
  // TODO
  std::string key(ckey);

  if(key == "verbosity"){
    // TODO
    int v = (long long) value;
    cc->verbosity = v;
  } else {
    cc->debug(NONE) << "Compiler key " << key << " not found." << std::endl;
    exit(1);
  }
}

void *getCallback(CC *cc, char *ckey){
  std::string key(ckey);

  if(key == "verbosity"){
    return (void *)(long long)cc->verbosity;
  } else {
    cc->debug(NONE) << "Compiler key " << key << " not found." << std::endl;
    exit(1);
  }
}

void *getParserCallback(CC *cc, char *ckey){
  std::string key(ckey);

  // TODO error handling
  cc->debug(LOW) << "Getting parser " << key << std::endl;
  return cc->parser.getParserWithKey(key);
}

void *registerParserCallback(CC *cc, char *ckey, char *name){
  // TODO error handling
  std::string key(ckey);
  std::string parserName(name);

  // top, type, value, statement, expression

  DynamicParseRule *drp = new DynamicParseRule(cc, parserName);
  
  if(key == "top"){
    cc->parser.registerTopRule(drp);
  } else if(key == "type"){
    cc->parser.registerTypeRule(drp);
  } else if(key == "value"){
    cc->parser.registerValueRule(drp);
  } else if(key == "statement"){
    cc->parser.registerStatementRule(drp);
  } else if(key == "expression"){
    cc->parser.registerExpressionRule(drp);
  } else {
    cc->debug(NONE) << "Unknown parser type in register" << std::endl;
    exit(1);
  }

  return 0;
}

void *parserParseCallback(ParseRule *pr, CC *cc, Source *s, int pos){
  // TODO error handling
  auto ans = pr->parse(cc, s, pos);
  if(!ans) return 0;
  ParseResult *parseResult = new ParseResult(ans);
  return parseResult;
}

void astSetCallback(AST *ast, char *ckey, void *value){
  // TODO error handling
  ast->set(std::string(ckey), value);
}

void astAddCallback(AST *ast, char *ckey, void *value){
  // TODO error handling
  ast->add(std::string(ckey), value);
}

void *parseResultGetCallback(ParseResult *pr, char *ckey){
  // TODO error handling
  return pr->token->get(CSTRTOSTR(ckey));
}

// ========== AST ==========

// Address
#include "ast/Address.h"
Address *createAddress(CC *cc, Expression *exp){
  return new Address(exp);
}

Expression *AddressGetExp(Address *a){
  return a->exp;
}

void AddressSetExp(Address *a, Expression *exp){
  a->exp = exp;
}

// Arg
#include "ast/Arg.h"
Arg *createArg(CC *cc){
  return new Arg();
}

char *ArgGetId(Arg *arg){
  return STRTOCSTR(arg->id);
}

Type *ArgGetType(Arg *arg){
  return arg->type;
}

void ArgSetId(Arg *arg, char *id){
  arg->id = CSTRTOSTR(id);
}

void ArgSetType(Arg *arg, Type *t){
  arg->type = t;
}

// Assign
#include "ast/Assign.h"
Assign *createAssign(CC *cc, Expression *base, Expression *exp){
  return new Assign(base, exp);
}
Expression *AssignGetBase(Assign *a){
  return a->baseExpr;
}
Expression *AssignGetExp(Assign *a){
  return a->expression;
}
void AssignSetBase(Assign *a, Expression *e){
  a->baseExpr = e;
}
void AssignSetExp(Assign *a, Expression *e){
  a->expression = e;
}

// BinOp
#include "ast/BinOp.h"
BinOp *createBinOp(CC *cc, Expression *lhs, char *op, Expression *rhs){
  return new BinOp(lhs, CSTRTOSTR(op), rhs);
}

Expression *BinOpGetLHS(BinOp *bo){
  return bo->lhs;
}
Expression *BinOpGetRHS(BinOp *bo){
  return bo->rhs;
}
char *BinOpGetOp(BinOp *bo){
  return STRTOCSTR(bo->op);
}

void BinOpSetLHS(BinOp *bo, Expression *lhs){
  bo->lhs = lhs;
}
void BinOpSetRHS(BinOp *bo, Expression *rhs){
  bo->rhs = rhs;
}
void BinOpSetOp(BinOp *bo, char *op){
  bo->op = CSTRTOSTR(op);
}

// Cast
#include "ast/Cast.h"
Cast *createCast(CC *cc, Expression *exp, Type *t){
  return new Cast(exp, t);
}
Expression *CastGetExp(Cast *c){
  return c->exp;
}
Type *CastGetType(Cast *c){
  return c->type_;
}
void CastSetExp(Cast *c, Expression *exp){
  c->exp = exp;
}
void CastSetType(Cast *c, Type *t){
  c->type_ = t;
}

// CodeBlock
#include "ast/CodeBlock.h"
CodeBlock *createCodeBlock(CC *cc){
  return new CodeBlock();
}

void CodeBlockAddStmt(CodeBlock *cb, Statement *stmt){
  cb->stmts.push_back(stmt);
}

Statement *CodeBlockGetStmt(CodeBlock *cb, int i){
  return cb->stmts[i];
}

// ConstBool
#include "ast/ConstBool.h"
ConstBool *createConstBool(CC *cc, bool b){
  return new ConstBool(b);
}
bool ConstBoolGet(ConstBool *b){
  return b->truth;
}
void ConstBoolSet(ConstBool *b, bool t){
  b->truth = t;
}
// ConstInt
#include "ast/ConstInt.h"
ConstInt *createConstInt(CC *cc, long long b){
  return new ConstInt(b);
}

long long ConstIntGet(ConstInt *b){
  return b->value;
}

void ConstIntSet(ConstInt *b, long long t){
  b->value = t;
}

// Define
#include "ast/Define.h"
Define *createDefine(CC *cc, char *id, Expression *exp, Type *t){
  return new Define(CSTRTOSTR(id), exp, t);
}
char *DefineGetId(Define *d){
  return STRTOCSTR(d->id);
}
Type *DefineGetType(Define *d){
  return d->type;
}
Expression *DefineGetExp(Define *d){
  return d->expression;
}

void DefineSetId(Define *d, char *id){
  d->id = CSTRTOSTR(id);
}
void DefineSetType(Define *d, Type *t){
  d->type = t;
}
void DefineSetExp(Define *d, Expression *exp){
  d->expression = exp;
}

// Directive
#include "ast/Directive.h"
Directive *createDirective(CC *cc, char *id, AST *ast){
  return new Directive(CSTRTOSTR(id), ast);
}
char *DirectiveGetId(Directive *d){
  return STRTOCSTR(d->id);
}
AST *DirectiveGetAST(Directive *d){
  return d->top;
}

void DirectiveSetId(Directive *d, char *id){
  d->id = CSTRTOSTR(id);
}

void DirectiveSetAST(Directive *d,AST *a){
  d->top = a;
}

// FCall
#include "ast/FCall.h"
FCall *createFCall(void *c){
  return new FCall();
}

char *FCallGetId(void *fc){
  auto fcall = (FCall*) fc;
  return (char *)fcall->id.c_str();
}

Expression *FCallGetArg(void *fc, int i){
  auto fcall = (FCall*) fc;
  return fcall->args.at(i);
}

void FCallSetId(void *fc, char *id){
  auto fcall = (FCall *) fc;
  fcall->id = std::string(id);
}

void FCallAddArg(void *fc, Expression *exp){
  auto fcall = (FCall *) fc;
  fcall->args.push_back(exp);
}

// FCallStmt


FCallStmt *createFCallStmt(void *c){
  return new FCallStmt();
}

char *FCallStmtGetId(void *fc){
  auto fcall = &((FCallStmt *) fc)->fc;
  return (char *)fcall->id.c_str();
}

Expression *FCallStmtGetArg(void *fc, int i){
  auto fcall = &((FCallStmt *) fc)->fc;
  return fcall->args.at(i);
}

void FCallStmtSetId(void *fc, char *id){
  auto fcall = &((FCallStmt *) fc)->fc;
  fcall->id = std::string(id);
}

void FCallStmtAddArg(void *fc, Expression *exp){
  auto fcall = &((FCallStmt *) fc)->fc;
  fcall->args.push_back(exp);
}
// Function
#include "ast/Function.h"
Function *createFunction(CC *cc){
  return new Function();
}
char *FunctionGetId(Function *f){
  return STRTOCSTR(f->id);
}
Type *FunctionGetReturntype(Function *f){
  return f->returnType;
}
Statement *FunctionGetStmt(Function *f, int i){
  return f->statements[i];
}
Arg *FunctionGetArg(Function *f, int i){
  return f->args[i];
}

void FunctionSetId(Function *f, char *id){
  f->id = CSTRTOSTR(id);
}
void FunctionSetReturnType(Function *f, Type *t){
  f->returnType = t;
}
void FunctionAddStmt(Function *f, Statement *s){
  f->statements.push_back(s);
}
void FunctionAddArg(Function *f, Arg *a){
  f->args.push_back(a);
}

// FunctionSig
#include "ast/FunctionSig.h"
FunctionSig *createFunctionSig(CC *cc){
  return new FunctionSig();
}
char *FunctionSigGetId(FunctionSig *f){
  return STRTOCSTR(f->id);
}
Type *FunctionSigGetReturnType(FunctionSig *f){
  return f->returnType;
}
Arg *FunctionSigGetArg(FunctionSig *f, int i){
  return f->args[i];
}
bool FunctionSigIsVararg(FunctionSig *f){
  return f->vararg;
}
void FunctionSigSetId(FunctionSig *f, char *id){
  f->id = CSTRTOSTR(id);
}
void FunctionSigSetReturnType(FunctionSig *f, Type *t){
  f->returnType = t;
}
void FunctionSigAddArg(FunctionSig *f, Arg *a){
  f->args.push_back(a);
}
void FunctionSigSetVararg(FunctionSig *f, bool b){
  f->vararg = b;
}

// FunctionType
#include "ast/FunctionType.h"
FunctionType *createFunctionType(CC *cc){
  return new FunctionType();
}
Type *FunctionTypeGetReturnType(FunctionType *f){
  return f->returnType;
}
Type *FunctionTypeGetArg(FunctionType *f, int i){
  return f->args[i];
}
bool FunctionTypeIsVararg(FunctionType *f){
  return f->vararg;
}

void FunctionTypeSetReturnType(FunctionType *f, Type *t){
  f->returnType = t;
}
void FunctionTypeAddArg(FunctionType *f, Type *a){
  f->args.push_back(a);
}
void FunctionTypeSetVararg(FunctionType *f, bool b){
  f->vararg = b;
}

// If
#include "ast/If.h"
If *createIf(CC *cc, Expression *exp, Statement *st1, Statement *st2){
  return new If(exp, st1, st2);
}
Expression *IfGetExp(If *i){
  return i->condition;
}
Statement *IfGetSt1(If *i){
  return i->st1;
}
Statement *IfGetSt2(If *i){
  return i->st2;
}
void IfSetExp(If *i, Expression *exp){
  i->condition = exp;
}
void IfSetSt1(If *i, Statement *st){
  i->st1 = st;
}
void IfSetSt2(If *i, Statement *st){
  i->st2 = st;
}

// Import
#include "ast/Import.h"
Import *createImport(CC *cc, char *path){
  return new Import(CSTRTOSTR(path));
}

char *ImportGetPath(Import *i){
  return STRTOCSTR(i->importPath);
}

void ImportSetPath(Import *i, char *p){
  i->importPath = CSTRTOSTR(p);
}

// Interface
#include "ast/Interface.h"
Interface *createInterface(CC *cc, char *id){
  return new Interface(CSTRTOSTR(id));
}
char *InterfaceGetId(Interface *i){
  return STRTOCSTR(i->id);
}
FunctionSig *InterfaceGetMethod(Interface *i, char *key){
  return i->methods[CSTRTOSTR(key)];
}

void InterfaceSetId(Interface *i, char *id){
  i->id = CSTRTOSTR(id);
}

void InterfaceSetMethod(Interface *i, char *key, FunctionSig *fs){
  i->methods[CSTRTOSTR(key)] = fs;
}

// MemAccess
#include "ast/MemAccess.h"
MemAccess *createMemAccess(CC *cc, Expression *exp, char *id){
  return new MemAccess(exp, CSTRTOSTR(id));
}

char *MemAccessGetId(MemAccess *m){
  return STRTOCSTR(m->id);
}

Expression *MemAccessGetExp(MemAccess *m){
  return m->exp;
}

void MemAccessSetId(MemAccess *m, char *id){
  m->id = CSTRTOSTR(id);
}

void MemAccessSetExp(MemAccess *m, Expression *e){
  m->exp = e;
}

// NamedType
#include "ast/NamedType.h"
NamedType *createNamedType(CC *cc, char *id){
  return new NamedType(CSTRTOSTR(id));
}

char *NamedTypeGetId(NamedType *n){
  return STRTOCSTR(n->id);
}

void NamedTypeSetId(NamedType *n, char *id){
  n->id = CSTRTOSTR(id);
}

// PointerType
#include "ast/PointerType.h"
PointerType *createPointerType(CC *cc, Type *t){
  return new PointerType(t);
}

Type *PointerTypeGetType(PointerType *pt){
  return pt->innerType;
}

void PointerTypeSetType(PointerType *pt, Type *t){
  pt->innerType = t;
}

// PreOp
#include "ast/PreOp.h"
PreOp *createPreOp(CC *cc, char *op, Expression *exp){
  return new PreOp(CSTRTOSTR(op), exp);
}
char *PreOpGetOp(PreOp *po){
  return STRTOCSTR(po->op);
}
Expression *PreOpGetExp(PreOp *po){
  return po->value;
}
void PreOpSetOp(PreOp *po, char *op){
  po->op = CSTRTOSTR(op);
}

void PreOpSetExp(PreOp *po, Expression *exp){
  po->value = exp;
}

// PrimitiveType
#include "ast/PrimitiveType.h"
#include "TypeEnum.h"
PrimitiveType *createPrimitiveType(CC *cc, char *ckey){
  return new PrimitiveType(typeEnumFromString(CSTRTOSTR(ckey)));
}
char *PrimitiveTypeGetKey(PrimitiveType *pt){
  std::string *ts = new std::string(typeEnumToString(pt->key));
  return (char *)ts->c_str();
}
void PrimitiveTypeSetKey(PrimitiveType *pt, char *key){
  pt->key = typeEnumFromString(CSTRTOSTR(key));
}

// PtrValue
#include "ast/PtrValue.h"
PtrValue *createPtrValue(CC *cc, Expression *exp){
  return new PtrValue(exp);
}
Expression *PtrValueGetPtr(PtrValue *pv){
  return pv->ptr;
}
void PtrValueSetPtr(PtrValue *pv, Expression *exp){
  pv->ptr = exp;
}

// Return
#include "ast/Return.h"
Return *createReturn(CC *cc, Expression *exp){
  return new Return(exp);
}
Expression *ReturnGetExp(Return *pv){
  return pv->value;
}
void ReturnSetExp(Return *pv, Expression *exp){
  pv->value = exp;
}

// Sizeof
#include "ast/Sizeof.h"
Sizeof *createSizeof(CC *cc, Type *t){
  return new Sizeof(t);
}
Type *SizeofGetType(Sizeof *s){
  return s->type_;
}
void SizeofSetType(Sizeof *s, Type *t){
  s->type_ = t;
}

// String
#include "ast/String.h"
String *createCString(CC *cc){
  return new String("");
}
char *CStringGetValue(String *s){
  return (char*)s->val.c_str();
}
void CStringSetValue(String *s, char *val){
  s->val = std::string(val);
}

// Struct
#include "ast/Struct.h"
StructType *createStruct(CC *cc, char *id){
  return new StructType(CSTRTOSTR(id));
}

char *StructGetId(StructType *st){
  return STRTOCSTR(st->id);
}
Define *StructGetMember(StructType *st, int i){
  return st->members[i];
}
void StructSetId(StructType *st, char *id){
  st->id = CSTRTOSTR(id);
}

void StructAddMember(StructType *st, Define *d){
  st->members.push_back(d);
}

// VariableValue
#include "ast/VariableValue.h"
VariableValue *createVariableValue(CC *cc, char *id){
  return new VariableValue(CSTRTOSTR(id));
}
char *VariableValueGetId(VariableValue *v){
  return STRTOCSTR(v->id);
}
void VariableValueSetId(VariableValue *v, char *id){
  v->id = CSTRTOSTR(id);
}

// While
#include "ast/While.h"
While *createWhile(CC *cc, Expression *cond, Statement *st){
  return new While(cond, st);
}

Expression *WhileGetCond(While *w){
  return w->condition;
}
Statement *WhileGetStmt(While *w){
  return w->st;
}
void WhileSetCond(While *w, Expression *cond){
  w->condition = cond;
}
void WhileSetStmt(While *w, Statement *stmt){
  w->st = stmt;
}

// TODO ensure CC is in all creates.
// TODO check all integers and long ong

#define REGISTER_CALLBACK(name, replace, cb)                                   \
  {                                                                            \
    llvm::Function *f = EE->FindFunctionNamed(name);                           \
    if (f) {                                                                   \
      llvm::Function *n = llvm::Function::Create(                              \
          f->getFunctionType(), llvm::Function::ExternalLinkage, replace,      \
          *this->llc->module);                                                 \
      auto bb = llvm::BasicBlock::Create(this->llc->context, "entry", n);      \
      llvm::IRBuilder<> builder(bb, bb->begin());                              \
      builder.SetInsertPoint(bb);                                              \
      auto fp = builder.CreateIntToPtr(                                        \
          llvm::ConstantInt::get(llvm::Type::getInt64Ty(llc->context),         \
                                 (uint64_t)cb, false),                         \
          f->getFunctionType()->getPointerTo());                               \
      std::vector<llvm::Value *> *args;                                        \
      args = new std::vector<llvm::Value *>();                                 \
      auto inargs = n->args();                                                 \
      for (auto &arg : inargs)                                                 \
        args->push_back(&arg);                                                 \
      builder.CreateRet(builder.CreateCall(f->getFunctionType(), fp, *args));  \
      f->replaceAllUsesWith(n);                                                \
      n->takeName(f);                                                          \
      f->eraseFromParent();                                                    \
    }                                                                          \
  }

void *CompileContext::getCompileObj(void *e){

  llvm::ExecutionEngine *EE = (llvm::ExecutionEngine*) e;
  
  if(compileObj) return compileObj;

  compileObj = this; // nothign is insie, so we don't care

  // Initialize compile functions in the module.

  REGISTER_CALLBACK("Compiler$exit", "Compiler$exit_replace", &exitCallback);
  REGISTER_CALLBACK("Compiler$set", "Compiler$set_replace", &setCallback);
  REGISTER_CALLBACK("Compiler$get", "Compiler$get_replace", &getCallback);
  REGISTER_CALLBACK("Compiler$getParser", "Compiler$getParser_replace", &getParserCallback);
  REGISTER_CALLBACK("Compiler$registerParser", "Compiler$registerParser_replace", &registerParserCallback);

  REGISTER_CALLBACK("Parser$parse", "Parser$parse_replace", &parserParseCallback);

  REGISTER_CALLBACK("AST$set", "AST$set_replace", &astSetCallback);
  REGISTER_CALLBACK("AST$add", "AST$add_replace", &astAddCallback);

  REGISTER_CALLBACK("ParseResult$get", "ParseResult$get_replace", &parseResultGetCallback);

  // Address
  REGISTER_CALLBACK("Compiler$createAddress", "Compiler$createAddress_replace",
                    &createAddress);
  REGISTER_CALLBACK("C_Address$getExp", "C_Address$getExp_replace",
                    & AddressGetExp);
  REGISTER_CALLBACK("C_Address$setExp", "C_Address$setExp_replace",
                    & AddressSetExp);

  // Arg
  REGISTER_CALLBACK("Compiler$createArg", "Compiler$createArg_replace",
                    &createArg);
  REGISTER_CALLBACK("C_Arg$getId", "C_Arg$getId_replace", & ArgGetId);
  REGISTER_CALLBACK("C_Arg$getType", "C_Arg$getType_replace", & ArgGetType);
  REGISTER_CALLBACK("C_Arg$setId", "C_Arg$setId_replace", & ArgSetId);
  REGISTER_CALLBACK("C_Arg$setType", "C_Arg$setType_replace", & ArgSetType);

  // Assign
  REGISTER_CALLBACK("Compiler$createAssign", "Compiler$createAssign_replace",
                    &createAssign);
  REGISTER_CALLBACK("C_Assign$getBase", "C_Assign$getBase_replace",
                    & AssignGetBase);
  REGISTER_CALLBACK("C_Assign$getExp", "C_Assign$getExp_replace",
                    & AssignGetExp);
  REGISTER_CALLBACK("C_Assign$setBase", "C_Assign$setBase_replace",
                    & AssignSetBase);
  REGISTER_CALLBACK("C_Assign$setExp", "C_Assign$setExp_replace",
                    & AssignSetExp);

  // BinOp
  REGISTER_CALLBACK("Compiler$createBinOp", "Compiler$createBinOp_replace",
                    &createBinOp);
  REGISTER_CALLBACK("C_BinOp$getLhs", "C_BinOp$getLhs_replace",
                    & BinOpGetLHS);
  REGISTER_CALLBACK("C_BinOp$getRhs", "C_BinOp$getRhs_replace",
                    & BinOpGetRHS);
  REGISTER_CALLBACK("C_BinOp$getOp", "C_BinOp$getOp_replace", & BinOpGetOp);
  REGISTER_CALLBACK("C_BinOp$setLHS", "C_BinOp$setLHS_replace",
                    & BinOpSetLHS);
  REGISTER_CALLBACK("C_BinOp$setRHS", "C_BinOp$setRHS_replace",
                    & BinOpSetRHS);
  REGISTER_CALLBACK("C_BinOp$setOp", "C_BinOp$setOp_replace", & BinOpSetOp);

  // Cast
  REGISTER_CALLBACK("Compiler$createCast", "Compiler$createCast_replace",
                    &createCast);
  REGISTER_CALLBACK("C_Cast$getType", "C_Cast$getType_replace",
                    & CastGetType);
  REGISTER_CALLBACK("C_Cast$getExp", "C_Cast$getExp_replace", & CastGetExp);
  REGISTER_CALLBACK("C_Cast$setType", "C_Cast$setType_replace",
                    & CastSetType);
  REGISTER_CALLBACK("C_Cast$setExp", "C_Cast$setExp_replace", & CastSetExp);

  // CodeBlock
  REGISTER_CALLBACK("Compiler$createCodeBlock",
                    "Compiler$createCodeBlock_replace",
                    &createCodeBlock);
  REGISTER_CALLBACK("C_CodeBlock$addStmt", "C_CodeBlock$addStmt_replace",
                    & CodeBlockAddStmt);
  REGISTER_CALLBACK("C_CodeBlock$getStmt", "C_CodeBlock$getStmt_replace",
                    & CodeBlockGetStmt);

  // ConstBool
  REGISTER_CALLBACK("Compiler$createConstBool",
                    "Compiler$createConstBool_replace",
                    &createConstBool);
  REGISTER_CALLBACK("C_ConstBool$get", "C_ConstBool$get_replace",
                    & ConstBoolGet);
  REGISTER_CALLBACK("C_ConstBool$set", "C_ConstBool$set_replace",
                    & ConstBoolSet);

  // ConstInt
  REGISTER_CALLBACK("Compiler$createConstInt",
                    "Compiler$createConstInt_replace", &createConstInt);
  REGISTER_CALLBACK("C_ConstInt$get", "C_ConstInt$get_replace",
                    & ConstIntGet);
  REGISTER_CALLBACK("C_ConstInt$set", "C_ConstInt$set_replace",
                    & ConstIntSet);

  // Define
  REGISTER_CALLBACK("Compiler$createDefine", "Compiler$createDefine_replace",
                    &createDefine);
  REGISTER_CALLBACK("C_Define$getId", "C_Define$getId_replace",
                    & DefineGetId);
  REGISTER_CALLBACK("C_Define$getType", "C_Define$getType_replace",
                    & DefineGetType);
  REGISTER_CALLBACK("C_Define$getExp", "C_Define$getExp_replace",
                    & DefineGetExp);
  REGISTER_CALLBACK("C_Define$setId", "C_Define$setId_replace",
                    & DefineSetId);
  REGISTER_CALLBACK("C_Define$setType", "C_Define$setType_replace",
                    & DefineSetType);
  REGISTER_CALLBACK("C_Define$setExp", "C_Define$setExp_replace",
                    & DefineSetExp);

  // Directive
  REGISTER_CALLBACK("Compiler$createDirective",
                    "Compiler$createDirective_replace",
                    &createDirective);
  REGISTER_CALLBACK("C_Directive$getId", "C_Directive$getId_replace",
                    & DirectiveGetId);
  REGISTER_CALLBACK("C_Directive$getAST", "C_Directive$getAST_replace",
                    & DirectiveGetAST);
  REGISTER_CALLBACK("C_Directive$setId", "C_Directive$setId_replace",
                    & DirectiveSetId);
  REGISTER_CALLBACK("C_Directive$setAST", "C_Directive$setAST_replace",
                    & DirectiveSetAST);

  // FCall
  REGISTER_CALLBACK("Compiler$createFCall", "Compiler$createFCall_replace",
                    &createFCall);
  REGISTER_CALLBACK("C_FCall$getId", "C_FCall$getId_replace", & FCallGetId);
  REGISTER_CALLBACK("C_FCall$getArg", "C_FCall$getArg_replace",
                    & FCallGetArg);
  REGISTER_CALLBACK("C_FCall$setId", "C_FCall$setId_replace", & FCallSetId);
  REGISTER_CALLBACK("C_FCall$addArg", "C_FCall$addArg_replace",
                    & FCallAddArg);

  // FCallStmt
  REGISTER_CALLBACK("Compiler$createFCallStmt",
                    "Compiler$createFCallStmt_replace",
                    &createFCallStmt);
  REGISTER_CALLBACK("C_FCallStmt$getId", "C_FCallStmt$getId_replace",
                    & FCallStmtGetId);
  REGISTER_CALLBACK("C_FCallStmt$getArg", "C_FCallStmt$getArg_replace",
                    & FCallStmtGetArg);
  REGISTER_CALLBACK("C_FCallStmt$setId", "C_FCallStmt$setId_replace",
                    & FCallStmtSetId);
  REGISTER_CALLBACK("C_FCallStmt$addArg", "C_FCallStmt$addArg_replace",
                    & FCallStmtAddArg);

  // Function
  REGISTER_CALLBACK("Compiler$createFunction",
                    "Compiler$createFunction_replace", &createFunction);
  REGISTER_CALLBACK("C_Function$getId", "C_Function$getId_replace",
                    & FunctionGetId);
  REGISTER_CALLBACK("C_Function$getReturntype",
                    "C_Function$getReturntype_replace",
                    & FunctionGetReturntype);
  REGISTER_CALLBACK("C_Function$getStmt", "C_Function$getStmt_replace",
                    & FunctionGetStmt);
  REGISTER_CALLBACK("C_Function$getArg", "C_Function$getArg_replace",
                    & FunctionGetArg);
  REGISTER_CALLBACK("C_Function$setId", "C_Function$setId_replace",
                    & FunctionSetId);
  REGISTER_CALLBACK("C_Function$setReturntype",
                    "C_Function$setReturntype_replace",
                    & FunctionSetReturnType);
  REGISTER_CALLBACK("C_Function$addStmt", "C_Function$addStmt_replace",
                    & FunctionAddStmt);
  REGISTER_CALLBACK("C_Function$addArg", "C_Function$addArg_replace",
                    & FunctionAddArg);

  // FunctionSig
  REGISTER_CALLBACK("Compiler$createFunctionSig",
                    "Compiler$createFunctionSig_replace",
                    &createFunctionSig);
  REGISTER_CALLBACK("C_FunctionSig$getId", "C_FunctionSig$getId_replace",
                    & FunctionSigGetId);
  REGISTER_CALLBACK("C_FunctionSig$getReturntype",
                    "C_FunctionSig$getReturntype_replace",
                    & FunctionSigGetReturnType);
  REGISTER_CALLBACK("C_FunctionSig$getArg", "C_FunctionSig$getArg_replace",
                    & FunctionSigGetArg);
  REGISTER_CALLBACK("C_FunctionSig$isVararg", "C_FunctionSig$isVararg_replace",
                    & FunctionSigIsVararg);
  REGISTER_CALLBACK("C_FunctionSig$setId", "C_FunctionSig$setId_replace",
                    & FunctionSigSetId);
  REGISTER_CALLBACK("C_FunctionSig$setReturntype",
                    "C_FunctionSig$setReturntype_replace",
                    & FunctionSigSetReturnType);
  REGISTER_CALLBACK("C_FunctionSig$addArg", "C_FunctionSig$addArg_replace",
                    & FunctionSigAddArg);
  REGISTER_CALLBACK("C_FunctionSig$setVararg",
                    "C_FunctionSig$setVararg_replace",
                    & FunctionSigSetVararg);

  // FunctionType
  REGISTER_CALLBACK("Compiler$createFunctionType",
                    "Compiler$createFunctionType_replace",
                    &createFunctionType);
  REGISTER_CALLBACK("C_FunctionType$getReturntype",
                    "C_FunctionType$getReturntype_replace",
                    & FunctionTypeGetReturnType);
  REGISTER_CALLBACK("C_FunctionType$getArg", "C_FunctionType$getArg_replace",
                    & FunctionTypeGetArg);
  REGISTER_CALLBACK("C_FunctionType$isVararg",
                    "C_FunctionType$isVararg_replace",
                    & FunctionTypeIsVararg);
  REGISTER_CALLBACK("C_FunctionType$setReturntype",
                    "C_FunctionType$setReturntype_replace",
                    & FunctionTypeSetReturnType);
  REGISTER_CALLBACK("C_FunctionType$addArg", "C_FunctionType$addArg_replace",
                    & FunctionTypeAddArg);
  REGISTER_CALLBACK("C_FunctionType$setVararg",
                    "C_FunctionType$setVararg_replace",
                    & FunctionTypeSetVararg);

  // If
  REGISTER_CALLBACK("Compiler$createIf", "Compiler$createIf_replace",
                    &createIf);
  REGISTER_CALLBACK("C_If$getExp", "C_If$getExp_replace", & IfGetExp);
  REGISTER_CALLBACK("C_If$getSt1", "C_If$getSt1_replace", & IfGetSt1);
  REGISTER_CALLBACK("C_If$getSt2", "C_If$getSt2_replace", & IfGetSt2);
  REGISTER_CALLBACK("C_If$setExp", "C_If$setExp_replace", & IfSetExp);
  REGISTER_CALLBACK("C_If$setSt1", "C_If$setSt1_replace", & IfSetSt1);
  REGISTER_CALLBACK("C_If$setSt2", "C_If$setSt2_replace", & IfSetSt2);

  // Import
  REGISTER_CALLBACK("Compiler$createImport", "Compiler$createImport_replace",
                    &createImport);
  REGISTER_CALLBACK("C_Import$getPath", "C_Import$getPath_replace",
                    & ImportGetPath);
  REGISTER_CALLBACK("C_Import$setPath", "C_Import$setPath_replace",
                    & ImportSetPath);

  // Interface
  REGISTER_CALLBACK("Compiler$createInterface",
                    "Compiler$createInterface_replace",
                    &createInterface);
  REGISTER_CALLBACK("C_Interface$getId", "C_Interface$getId_replace",
                    & InterfaceGetId);
  REGISTER_CALLBACK("C_Interface$getMethod", "C_Interface$getMethod_replace",
                    & InterfaceGetMethod);
  REGISTER_CALLBACK("C_Interface$setId", "C_Interface$setId_replace",
                    & InterfaceSetId);
  REGISTER_CALLBACK("C_Interface$setMethod", "C_Interface$setMethod_replace",
                    & InterfaceSetMethod);

  // MemAccess
  REGISTER_CALLBACK("Compiler$createMemAccess",
                    "Compiler$createMemAccess_replace",
                    &createMemAccess);
  REGISTER_CALLBACK("C_MemAccess$getId", "C_MemAccess$getId_replace",
                    & MemAccessGetId);
  REGISTER_CALLBACK("C_MemAccess$getExp", "C_MemAccess$getExp_replace",
                    & MemAccessGetExp);
  REGISTER_CALLBACK("C_MemAccess$setId", "C_MemAccess$setId_replace",
                    & MemAccessSetId);
  REGISTER_CALLBACK("C_MemAccess$setExp", "C_MemAccess$setExp_replace",
                    & MemAccessSetExp);

  // NamedType
  REGISTER_CALLBACK("Compiler$createNamedType",
                    "Compiler$createNamedType_replace",
                    &createNamedType);
  REGISTER_CALLBACK("C_NamedType$getId", "C_NamedType$getId_replace",
                    & NamedTypeGetId);
  REGISTER_CALLBACK("C_NamedType$setId", "C_NamedType$setId_replace",
                    & NamedTypeSetId);

  // PointerType
  REGISTER_CALLBACK("Compiler$createPointerType",
                    "Compiler$createPointerType_replace",
                    &createPointerType);
  REGISTER_CALLBACK("C_PointerType$getType", "C_PointerType$getType_replace",
                    & PointerTypeGetType);
  REGISTER_CALLBACK("C_PointerType$setType", "C_PointerType$setType_replace",
                    & PointerTypeSetType);

  // PreOp
  REGISTER_CALLBACK("Compiler$createPreOp", "Compiler$createPreOp_replace",
                    &createPreOp);
  REGISTER_CALLBACK("C_PreOp$getOp", "C_PreOp$getOp_replace", & PreOpGetOp);
  REGISTER_CALLBACK("C_PreOp$getExp", "C_PreOp$getExp_replace",
                    & PreOpGetExp);
  REGISTER_CALLBACK("C_PreOp$setOp", "C_PreOp$setOp_replace", & PreOpSetOp);
  REGISTER_CALLBACK("C_PreOp$setExp", "C_PreOp$setExp_replace",
                    & PreOpSetExp);

  // TODO enums?
  // PrimitiveType
  REGISTER_CALLBACK("Compiler$createPrimitiveType",
                    "Compiler$createPrimitiveType_replace",
                    &createPrimitiveType);
  REGISTER_CALLBACK("C_PrimitiveType$getKey", "C_PrimitiveType$getKey_replace",
                    & PrimitiveTypeGetKey);
  REGISTER_CALLBACK("C_PrimitiveType$setKey", "C_PrimitiveType$setKey_replace",
                    & PrimitiveTypeSetKey);

  // PtrValue
  REGISTER_CALLBACK("Compiler$createPtrValue",
                    "Compiler$createPtrValue_replace", &createPtrValue);
  REGISTER_CALLBACK("C_PtrValue$getPtr", "C_PtrValue$getPtr_replace",
                    & PtrValueGetPtr);
  REGISTER_CALLBACK("C_PtrValue$setPtr", "C_PtrValue$setPtr_replace",
                    & PtrValueSetPtr);

  // Return
  REGISTER_CALLBACK("Compiler$createReturn", "Compiler$createReturn_replace",
                    &createReturn);
  REGISTER_CALLBACK("C_Return$getExp", "C_Return$getExp_replace",
                    & ReturnGetExp);
  REGISTER_CALLBACK("C_Return$setExp", "C_Return$setExp_replace",
                    & ReturnSetExp);

  // Sizeof
  REGISTER_CALLBACK("Compiler$createSizeof", "Compiler$createSizeof_replace",
                    &createSizeof);
  REGISTER_CALLBACK("C_Sizeof$getType", "C_Sizeof$getType_replace",
                    & SizeofGetType);
  REGISTER_CALLBACK("C_Sizeof$setType", "C_Sizeof$setType_replace",
                    & SizeofSetType);

  // CString
  REGISTER_CALLBACK("Compiler$createCString", "Compiler$createCString_replace",
                    &createCString);
  REGISTER_CALLBACK("C_CString$getValue", "C_CString$getValue_replace",
                    & CStringGetValue);
  REGISTER_CALLBACK("C_CString$setValue", "C_CString$setValue_replace",
                    & CStringSetValue);

  // Struct
  REGISTER_CALLBACK("Compiler$createStruct", "Compiler$createStruct_replace",
                    &createStruct);
  REGISTER_CALLBACK("C_Struct$getId", "C_Struct$getId_replace",
                    & StructGetId);
  REGISTER_CALLBACK("C_Struct$getMember", "C_Struct$getMember_replace",
                    & StructGetMember);
  REGISTER_CALLBACK("C_Struct$setId", "C_Struct$setId_replace",
                    & StructSetId);
  REGISTER_CALLBACK("C_Struct$addMember", "C_Struct$addMember_replace",
                    & StructAddMember);
  // TODO adding methods?
  // TODO interfaces and structs

  // VariableValue
  REGISTER_CALLBACK("Compiler$createVariableValue",
                    "Compiler$createVariableValue_replace",
                    &createVariableValue);
  REGISTER_CALLBACK("C_VariableValue$getId", "C_VariableValue$getId_replace",
                    & VariableValueGetId);
  REGISTER_CALLBACK("C_VariableValue$setId", "C_VariableValue$setId_replace",
                    & VariableValueSetId);

  // While
  REGISTER_CALLBACK("Compiler$createWhile", "Compiler$createWhile_replace",
                    &createWhile);
  REGISTER_CALLBACK("C_While$getCond", "C_While$getCond_replace",
                    & WhileGetCond);
  REGISTER_CALLBACK("C_While$getStmt", "C_While$getStmt_replace",
                    & WhileGetStmt);
  REGISTER_CALLBACK("C_While$setCond", "C_While$setCond_replace",
                    & WhileSetCond);
  REGISTER_CALLBACK("C_While$setStmt", "C_While$setStmt_replace",
                    & WhileSetStmt);

  return compileObj;
}
