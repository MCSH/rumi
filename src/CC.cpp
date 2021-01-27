#include "LLContext.h"
#include "ast/Named.h"
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
    } else if(arg == "-r"){
      this->removeMeta = true;
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
  } else if(key == "removeMeta"){
    cc->removeMeta = (bool) value;
  } else {
    cc->debug(NONE) << "Compiler key " << key << " not found." << std::endl;
    exit(1);
  }
}

void *getCallback(CC *cc, char *ckey){
  std::string key(ckey);

  if(key == "verbosity"){
    return (void *)(long long)cc->verbosity;
  } else if(key == "removeMeta"){
    return (void*) cc->removeMeta;
  } else {
    cc->debug(NONE) << "Compiler key " << key << " not found." << std::endl;
    exit(1);
  }
}

void CompilerSetFMeta(CC *cc, char *key){
  cc->metaFunctions.insert(std::string(key));
}

void *registerParserCallback(CC *cc, char *ckey, char *name){
  // TODO error handling
  std::string key(ckey);
  std::string parserName(name);

  // top, type, value, statement, expression

  DynamicParseRule *drp = new DynamicParseRule(cc, parserName);
  
  if(key == "top"){
    cc->parser.registerTopRule(drp, true);
  } else if(key == "type"){
    cc->parser.registerTypeRule(drp, true);
  } else if(key == "value"){
    cc->parser.registerValueRule(drp, true);
  } else if(key == "statement"){
    cc->parser.registerStatementRule(drp, true);
  } else if(key == "expression"){
    cc->parser.registerExpressionRule(drp, true);
  } else {
    cc->debug(NONE) << "Unknown parser type in register" << std::endl;
    exit(1);
  }

  cc->metaFunctions.insert(parserName + "$parse");
  cc->metaFunctions.insert(parserName + "$genAST");

  return 0;
}

#include "parser/DynamicParseRule.h"

class DynamicToken: public Token{
public:
  void *val;
  virtual std::string desc() override{return "";}
  virtual AST* toAST(CC *cc) override{return 0;}
};

ParseResult *CompilerCreateDynamicParseResult(CC *cc){
  return new ParseResult(new DynamicToken());
}

void DynamicParseResultSet(ParseResult *p, void *v){
  if(!p) return ;
  auto dp = (DynamicToken *) p->token;
  dp -> val = v;
}

void *DynamicParseResultGet(ParseResult *p){
  if(!p) return 0;
  auto dp = (DynamicToken *) p->token;
  return dp->val;
}

void ParseResultCopyStart(ParseResult *p1, ParseResult *p2){
  if(!p1 || !p2) return ;
  p1->token->s = p2->token->s;
  p1->token->spos = p2->token->spos;
}

void ParseResultCopyEnd(ParseResult *p1, ParseResult *p2){
  if(!p1 || !p2) return ;
  p1->token->s = p2->token->s;
  p1->token->epos = p2->token->epos;
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
  if(i >= cb->stmts.size()) return 0;
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
FCall *createFCall(CC *cc){
  return new FCall();
}

char *FCallGetId(void *fc){
  auto fcall = (FCall*) fc;
  return (char *)fcall->id.c_str();
}

Expression *FCallGetArg(void *fc, int i){
  auto fcall = (FCall*) fc;
  if(i >= fcall->args.size()) return 0;
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


FCallStmt *createFCallStmt(CC *cc){
  return new FCallStmt();
}

char *FCallStmtGetId(void *fc){
  auto fcall = &((FCallStmt *) fc)->fc;
  return (char *)fcall->id.c_str();
}

Expression *FCallStmtGetArg(void *fc, int i){
  auto fcall = &((FCallStmt *) fc)->fc;
  if(i >= fcall->args.size()) return 0;
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
  if(i >= f->statements.size()) return 0;
  return f->statements[i];
}
Arg *FunctionGetArg(Function *f, int i){
  if(i >= f->args.size()) return 0;
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
  if(i >= f->args.size()) return 0;
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
  if(i >= f->args.size()) return 0;
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
  auto ans = i->methods.find(CSTRTOSTR(key));
  if(ans == i->methods.end()) return 0;
  return ans->second;
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

// Method
#include "ast/Method.h"
Method *createMethod(CC *cc, char *sname, char *mname, Function *f){
  return new Method(CSTRTOSTR(sname), CSTRTOSTR(mname), f);
}
char *MethodGetStructName(Method *m){
  return STRTOCSTR(m->structName);
}
char *MethodGetMethodName(Method *m){
  return STRTOCSTR(m->methodName);
}
Function *MethodGetFunction(Method *m){
  return m->f;
}

void MethodSetStructName(Method *m, char *name){
  m->structName = CSTRTOSTR(name);
}
void MethodSetMethodName(Method *m, char *name){
  m->methodName = CSTRTOSTR(name);
}
void MethodSetFunction(Method *m, Function *f){
  m->f = f;
}

// MethodCall
#include "ast/MethodCall.h"
MethodCall *createMethodCall(CC *cc){
  return new MethodCall();
}
MemAccess *MethodCallGetExp(MethodCall *mc){
  return mc->exp;
}
Expression *MethodCallGetArg(MethodCall *mc, int i){
  if(i >= mc->args.size()) return 0;
  return mc->args[i];
}
void MethodCallSetExp(MethodCall *mc, MemAccess *m){
  mc->exp = m;
}
void MethodCallAddArg(MethodCall *mc, int i, Expression  *e){
  mc->args.push_back(e);
}

MethodCallStmt *createMethodCallStmt(CC *cc){
  return new MethodCallStmt(new MethodCall());
}
MemAccess *MethodCallStmtGetExp(MethodCallStmt *mc){
  return mc->mc->exp;
}
Expression *MethodCallStmtGetArg(MethodCallStmt *mc, int i){
  if(i >= mc->mc->args.size()) return 0;
  return mc->mc->args[i];
}
void MethodCallStmtSetExp(MethodCallStmt *mc, MemAccess *m){
  mc->mc->exp = m;
}
void MethodCallStmtAddArg(MethodCallStmt *mc, int i, Expression  *e){
  mc->mc->args.push_back(e);
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
  if(i >= st->members.size()) return 0;
  return st->members[i];
}
void StructSetId(StructType *st, char *id){
  st->id = CSTRTOSTR(id);
}

void StructAddMember(StructType *st, Define *d){
  st->members.push_back(d);
}
void StructAddMethod(StructType *st, CC *cc, Method *m){
  st->addMethod(cc, m);
}

StructType *CompilerGetStruct(CC *cc, char *id){
  Named *n = cc->lookup(CSTRTOSTR(id));
  if(!n) return 0;
  return dynamic_cast<StructType*>(n->type);
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

// == AST Baes ==

char *C_TypeToString(Type *t){
  if(!t) return 0;
  return STRTOCSTR(t->toString());
}

// === Parser
#include "parser/AddressParser.h"
AddressParser * CompilerGetAddressParser(CC *cc){
  return new AddressParser();
}
ParseResult *CP_AddressParserParse(AddressParser *p, CC *cc, Source *s, int pos){
  auto ans = p->parse(cc, s, pos); return ans?new ParseResult(ans):0;
}
ParseResult *CP_AddressParserParseAfter(AddressParser *p, ParseResult *pr){
  if(!pr) return 0;
  auto ans = *pr >> *p;
  if(!ans) return 0;
  Token * a = ((TupleToken*) ans.token)->t2;
  return new ParseResult(a);
}
ParseResult *CP_AddressParser_ParseResultGetExp(ParseResult *p){
  if(!p) return 0;
  AddressToken *at = (AddressToken *)p->token;
  return new ParseResult(at->exp);
}

#include "parser/AssignParser.h"
AssignParser *CompilerGetAssignParser(CC *cc) { return new AssignParser(); }
ParseResult *CP_AssignParserParse(AssignParser *p, CC *cc, Source *s, int pos) {
  auto ans = p->parse(cc, s, pos); return ans?new ParseResult(ans):0;
}
ParseResult *CP_AssignParserParseAfter(AssignParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
ParseResult *CP_AssignParser_ParseResultGetExp(ParseResult *p){
  if(!p) return 0;
  AssignToken *at = (AssignToken *) p->token;
  return new ParseResult(at->expr);
}
ParseResult *CP_AssignParser_ParseResultGetValue(ParseResult *p){
  if(!p) return 0;
  AssignToken *at = (AssignToken *) p->token;
  return new ParseResult(at->value);
}

#include "parser/BinOpParser.h"
BinOpParser *CompilerGetBinOpParser(CC *cc) { return new BinOpParser(); }
ParseResult *CP_BinOpParserParse(BinOpParser *p, CC *cc, Source *s, int pos) {
  auto ans = p->parse(cc, s, pos); return ans?new ParseResult(ans):0;
}
ParseResult *CP_BinOpParserParseAfter(BinOpParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
char *CP_BinOpParser_ParseResultGetOp(ParseResult *p){
  if(!p) return 0;
  BinOpToken *bo = (BinOpToken *) p->token;
  return STRTOCSTR(symbolDesc(bo->op));
}
ParseResult *CP_BinOpParser_ParseResultGetlhs(ParseResult *p){
  if(!p) return 0;
  BinOpToken *bo = (BinOpToken *) p->token;
  return new ParseResult(bo->lhs);
}
ParseResult *CP_BinOpParser_ParseResultGetrhs(ParseResult *p){
  if(!p) return 0;
  BinOpToken *bo = (BinOpToken *) p->token;
  return new ParseResult(bo->rhs);
}
int CP_BinOpParser_ParseResultGetprec(ParseResult *p){
  if(!p) return 0;
  BinOpToken *bo = (BinOpToken *) p->token;
  return bo->prec;
}

#include "parser/BoolValueParser.h"
BoolValueParser *CompilerGetBoolValueParser(CC *cc) {
  return new BoolValueParser();
}
ParseResult *CP_BoolValueParserParse(BoolValueParser *p, CC *cc, Source *s,
                                     int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_BoolValueParserParseAfter(BoolValueParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
bool CP_BoolValueParser_ParseResultGetTruth(ParseResult *p) {
  if (!p)
    return 0;
  BoolToken *bt = (BoolToken *)p->token;
  return bt->truth;
}

#include "parser/CastExprParser.h"
CastExprParser *CompilerGetCastExprParser(CC *cc) {
  return new CastExprParser();
}
ParseResult *CP_CastExprParserParse(CastExprParser *p, CC *cc, Source *s,
                                    int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_CastExprParserParseAfter(CastExprParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
ParseResult *CP_CastExprParser_ParseResultGetExp(ParseResult *p) {
  if (!p)
    return 0;
  CastExprToken *c = (CastExprToken *)p->token;
  return new ParseResult(c->exp);
}
ParseResult *CP_CastExprParser_ParseResultGetType(ParseResult *p) {
  if (!p)
    return 0;
  CastExprToken *c = (CastExprToken *)p->token;
  return new ParseResult(c->type);
}

#include "parser/DefineParser.h"
DefineParser *CompilerGetDefineParser(CC *cc) { return new DefineParser(); }
ParseResult *CP_DefineParserParse(DefineParser *p, CC *cc, Source *s, int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_DefineParserParseAfter(DefineParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
char *CP_DefineParser_ParseResultGetId(ParseResult *p) {
  if (!p)
    return 0;
  DefineToken *d = (DefineToken *)p->token;
  return STRTOCSTR(d->id);
}
ParseResult *CP_DefineParser_ParseResultGetType(ParseResult *p) {
  if (!p)
    return 0;
  DefineToken *d = (DefineToken *)p->token;
  if (!d->type)
    return 0;
  return new ParseResult(d->type);
}
ParseResult *CP_DefineParser_ParseResultGetValue(ParseResult *p) {
  if (!p)
    return 0;
  DefineToken *d = (DefineToken *)p->token;
  if (!d->value)
    return 0;
  return new ParseResult(d->value);
}

#include "parser/DirectiveParser.h"
DirectiveParser *CompilerGetDirectiveParser(CC *cc) {
  return new DirectiveParser();
}
ParseResult *CP_DirectiveParserParse(DirectiveParser *p, CC *cc, Source *s,
                                     int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_DirectiveParserParseAfter(DirectiveParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
char *CP_DirectiveParser_ParseResultGetId(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (DirectiveToken *)p->token;
  return STRTOCSTR(t->id);
}
ParseResult *CP_DirectiveParser_ParseResultGetTop(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (DirectiveToken *)p->token;
  return new ParseResult(t->top);
}

#include "parser/ExpressionParser.h"
ExpressionParser *CompilerGetExpressionParser(CC *cc) {
  return new ExpressionParser();
}
ParseResult *CP_ExpressionParserParse(ExpressionParser *p, CC *cc, Source *s,
                                      int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_ExpressionParserParseAfter(ExpressionParser *p,
                                           ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}

#include "parser/FCallParser.h"
FCallParser *CompilerGetFCallParser(CC *cc) { return new FCallParser(); }
ParseResult *CP_FCallParserParse(FCallParser *p, CC *cc, Source *s, int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_FCallParserParseAfter(FCallParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
char *CP_FCallParser_ParseResultGetId(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (FCallToken *)p->token;
  return STRTOCSTR(t->id);
}
ParseResult *CP_FCallParser_ParseResultGetArg(ParseResult *p, int i) {
  if (!p)
    return 0;
  auto t = (FCallToken *)p->token;
  if (i >= t->args.size())
    return 0;
  return new ParseResult(t->args[i]);
}

#include "parser/FCallStmtParser.h"
FCallStmtParser *CompilerGetFCallStmtParser(CC *cc) {
  return new FCallStmtParser();
}
ParseResult *CP_FCallStmtParserParse(FCallStmtParser *p, CC *cc, Source *s,
                                     int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_FCallStmtParserParseAfter(FCallStmtParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
char *CP_FCallStmtParser_ParseResultGetId(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (FCallStmtToken *)p->token;
  return STRTOCSTR(t->id);
}
ParseResult *CP_FCallStmtParser_ParseResultGetArg(ParseResult *p, int i) {
  if (!p)
    return 0;
  auto t = (FCallStmtToken *)p->token;
  if (i >= t->args.size())
    return 0;
  return new ParseResult(t->args[i]);
}

#include "parser/FunctionParser.h"
FunctionParser *CompilerGetFunctionParser(CC *cc) {
  return new FunctionParser();
}
ParseResult *CP_FunctionParserParse(FunctionParser *p, CC *cc, Source *s,
                                    int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_FunctionParserParseAfter(FunctionParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
ParseResult *CP_FunctionParser_ParseResultGetReturnType(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (FunctionBodyToken *)p->token;
  if (!t->rt)
    return 0;
  return new ParseResult(t->rt);
}
ParseResult *CP_FunctionParser_ParseResultGetArg(ParseResult *p, int i) {
  if (!p)
    return 0;
  auto t = (FunctionBodyToken *)p->token;
  if (i >= t->args.size())
    return 0;
  return new ParseResult(t->args[i]);
}
ParseResult *CP_FunctionParser_ParseResultGetStmt(ParseResult *p, int i) {
  if (!p)
    return 0;
  auto t = (FunctionBodyToken *)p->token;
  if (i >= t->statements.size())
    return 0;
  return new ParseResult(t->statements[i]);
}

#include "parser/FunctionSigParser.h"
FunctionSigParser *CompilerGetFunctionSigParser(CC *cc) {
  return new FunctionSigParser();
}
ParseResult *CP_FunctionSigParserParse(FunctionSigParser *p, CC *cc, Source *s,
                                       int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_FunctionSigParserParseAfter(FunctionSigParser *p,
                                            ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
ParseResult *CP_FunctionSigParser_ParseResultGetReturnType(ParseResult *p) {
  if (!p)
    return 0;
  auto a = (FunctionSigToken *)p->token;
  if (!a->returnType)
    return 0;
  return new ParseResult(a->returnType);
}
ParseResult *CP_FunctionSigParser_ParseResultGetArg(ParseResult *p, int i) {
  if (!p)
    return 0;
  auto a = (FunctionSigToken *)p->token;
  if (i >= a->args.size())
    return 0;
  return new ParseResult(a->args[i]);
}

#include "parser/FunctionTypeParser.h"
FunctionTypeParser *CompilerGetFunctionTypeParser(CC *cc) {
  return new FunctionTypeParser();
}
ParseResult *CP_FunctionTypeParserParse(FunctionTypeParser *p, CC *cc,
                                        Source *s, int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_FunctionTypeParserParseAfter(FunctionTypeParser *p,
                                             ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
ParseResult *CP_FunctionTypeParser_ParseResultGetReturnType(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (FunctionTypeToken *)p->token;
  if (!t->rt)
    return 0;
  return new ParseResult(t->rt);
}
ParseResult *CP_FunctionTypeParser_ParseResultGetArg(ParseResult *p, int i) {
  if (!p)
    return 0;
  auto t = (FunctionTypeToken *)p->token;
  if (i >= t->args.size())
    return 0;
  return new ParseResult(t->args[i]);
}

#include "parser/IdParser.h"
IdParser *CompilerGetIdParser(CC *cc) { return new IdParser(); }
ParseResult *CP_IdParserParse(IdParser *p, CC *cc, Source *s, int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_IdParserParseAfter(IdParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
char *CP_IdParser_ParseResultGetId(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (IdToken *)p->token;
  return STRTOCSTR(t->id);
}

#include "parser/IfParser.h"
IfParser *CompilerGetIfParser(CC *cc) { return new IfParser(); }
ParseResult *CP_IfParserParse(IfParser *p, CC *cc, Source *s, int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_IfParserParseAfter(IfParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
ParseResult *CP_IfParser_ParseResultGetValue(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (IfToken *)p->token;
  return new ParseResult(t->value);
}
ParseResult *CP_IfParser_ParseResultGetSt1(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (IfToken *)p->token;
  return new ParseResult(t->st1);
}
ParseResult *CP_IfParser_ParseResultGetSt2(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (IfToken *)p->token;
  if (!t->st2)
    return 0;
  return new ParseResult(t->st2);
}

#include "parser/ImportParser.h"
ImportParser *CompilerGetImportParser(CC *cc) { return new ImportParser(); }
ParseResult *CP_ImportParserParse(ImportParser *p, CC *cc, Source *s, int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_ImportParserParseAfter(ImportParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
char *CP_ImportParser_ParseResultGetPath(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (ImportToken *)p->token;
  return STRTOCSTR(t->importPath);
}

#include "parser/InterfaceParser.h"
InterfaceParser *CompilerGetInterfaceParser(CC *cc) {
  return new InterfaceParser();
}
ParseResult *CP_InterfaceParserParse(InterfaceParser *p, CC *cc, Source *s,
                                     int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_InterfaceParserParseAfter(InterfaceParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
char *CP_InterfaceParser_ParseResultGetId(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (InterfaceToken *)p->token;
  return STRTOCSTR(t->id);
}
ParseResult *CP_InterfaceParser_ParseResultGetMethod(ParseResult *p, int i) {
  if (!p)
    return 0;
  auto t = (InterfaceToken *)p->token;
  if (i >= t->methods.size())
    return 0;
  return new ParseResult(t->methods[i]);
}

#include "parser/KeywordParser.h"
KeywordParser *CompilerGetKeywordParser(CC *cc) { return new KeywordParser(); }
ParseResult *CP_KeywordParserParse(KeywordParser *p, CC *cc, Source *s,
                                   int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_KeywordParserParseAfter(KeywordParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
char *CP_KeywordParser_ParseResultGetKeyword(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (KeywordToken *)p->token;
  return STRTOCSTR(t->desc());
}

#include "parser/MemAccessParser.h"
MemAccessParser *CompilerGetMemAccessParser(CC *cc) {
  return new MemAccessParser();
}
ParseResult *CP_MemAccessParserParse(MemAccessParser *p, CC *cc, Source *s,
                                     int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_MemAccessParserParseAfter(MemAccessParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
char *CP_MemAccessParser_ParseResultGetId(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (MemAccessToken *)p->token;
  return STRTOCSTR(t->id);
}
ParseResult *CP_MemAccessParser_ParseResultGetExp(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (MemAccessToken *)p->token;
  return new ParseResult(t->expression);
}

#include "parser/MethodCallParser.h"
MethodCallParser *CompilerGetMethodCallParser(CC *cc) {
  return new MethodCallParser();
}
ParseResult *CP_MethodCallParserParse(MethodCallParser *p, CC *cc, Source *s,
                                      int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_MethodCallParserParseAfter(MethodCallParser *p,
                                           ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
ParseResult *CP_MethodCallParser_ParseResultGetExp(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (MethodCallToken *)p->token;
  return new ParseResult(t->exp);
}
ParseResult *CP_MethodCallParser_ParseResultGetArg(ParseResult *p, int i) {
  if (!p)
    return 0;
  auto t = (MethodCallToken *)p->token;
  if (i >= t->args.size())
    return 0;
  return new ParseResult(t->args[i]);
}

#include "parser/MethodCallStmtParser.h"
MethodCallStmtParser *CompilerGetMethodCallStmtParser(CC *cc) {
  return new MethodCallStmtParser();
}
ParseResult *CP_MethodCallStmtParserParse(MethodCallStmtParser *p, CC *cc,
                                          Source *s, int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_MethodCallStmtParserParseAfter(MethodCallStmtParser *p,
                                               ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
ParseResult *CP_MethodCallStmtParser_ParseResultGetExp(ParseResult *p) {
  if (!p)
    return 0;
  auto tt = (MethodCallStmtToken *)p->token;
  auto t = (MethodCallToken *)tt->mct;
  return new ParseResult(t->exp);
}
ParseResult *CP_MethodCallStmtParser_ParseResultGetArg(ParseResult *p, int i) {
  if (!p)
    return 0;
  auto tt = (MethodCallStmtToken *)p->token;
  auto t = (MethodCallToken *)tt->mct;
  if (i >= t->args.size())
    return 0;
  return new ParseResult(t->args[i]);
}

#include "parser/MethodParser.h"
MethodParser *CompilerGetMethodParser(CC *cc) { return new MethodParser(); }
ParseResult *CP_MethodParserParse(MethodParser *p, CC *cc, Source *s, int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_MethodParserParseAfter(MethodParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
char *CP_MethodParser_ParseResultGetStructName(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (MethodToken *)p->token;
  return STRTOCSTR(t->structName);
}
char *CP_MethodParser_ParseResultGetMethodName(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (MethodToken *)p->token;
  return STRTOCSTR(t->methodName);
}
ParseResult *CP_MethodParser_ParseResultGetf(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (MethodToken *)p->token;
  return new ParseResult(t->f);
}

#include "parser/NamedTypeParser.h"
NamedTypeParser *CompilerGetNamedTypeParser(CC *cc) {
  return new NamedTypeParser();
}
ParseResult *CP_NamedTypeParserParse(NamedTypeParser *p, CC *cc, Source *s,
                                     int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_NamedTypeParserParseAfter(NamedTypeParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
char *CP_NamedTypeParser_ParseResultGetId(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (NamedTypeToken *)p->token;
  return STRTOCSTR(t->typeId);
}

#include "parser/NumberParser.h"
NumberParser *CompilerGetNumberParser(CC *cc) { return new NumberParser(); }
ParseResult *CP_NumberParserParse(NumberParser *p, CC *cc, Source *s, int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_NumberParserParseAfter(NumberParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
int CP_NumberParser_ParseResultGetValue(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (NumberToken *)p->token;
  return t->val;
}

#include "parser/ParenParser.h"
ParenParser *CompilerGetParenParser(CC *cc) { return new ParenParser(); }
ParseResult *CP_ParenParserParse(ParenParser *p, CC *cc, Source *s, int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_ParenParserParseAfter(ParenParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
ParseResult *CP_ParenParser_ParseResultGetExp(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (ParenToken *)p->token;
  return new ParseResult(t->exp);
}

#include "parser/PointerTypeParser.h"
PointerTypeParser *CompilerGetPointerTypeParser(CC *cc) {
  return new PointerTypeParser();
}
ParseResult *CP_PointerTypeParserParse(PointerTypeParser *p, CC *cc, Source *s,
                                       int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_PointerTypeParserParseAfter(PointerTypeParser *p,
                                            ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
ParseResult *CP_PointerTypeParser_ParseResultGetInnerType(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (PointerTypeToken *)p->token;
  return new ParseResult(t->innerType);
}

#include "parser/PreOpParser.h"
PreOpParser *CompilerGetPreOpParser(CC *cc) { return new PreOpParser(); }
ParseResult *CP_PreOpParserParse(PreOpParser *p, CC *cc, Source *s, int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_PreOpParserParseAfter(PreOpParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
char *CP_PreOpParser_ParseResultGetOp(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (PreOpToken *)p->token;
  return STRTOCSTR(t->op);
}
ParseResult *CP_PreOpParser_ParseResultGetValue(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (PreOpToken *)p->token;
  return new ParseResult(t->value);
}

#include "parser/PrimitiveTypeParser.h"
PrimitiveTypeParser *CompilerGetPrimitiveTypeParser(CC *cc) {
  return new PrimitiveTypeParser();
}
ParseResult *CP_PrimitiveTypeParserParse(PrimitiveTypeParser *p, CC *cc,
                                         Source *s, int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_PrimitiveTypeParserParseAfter(PrimitiveTypeParser *p,
                                              ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
char *CP_PrimitiveTypeParser_ParseResultGetKey(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (TypeToken *)p->token;
  return STRTOCSTR(t->desc());
}

#include "parser/PtrValueParser.h"
PtrValueParser *CompilerGetPtrValueParser(CC *cc) {
  return new PtrValueParser();
}
ParseResult *CP_PtrValueParserParse(PtrValueParser *p, CC *cc, Source *s,
                                    int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_PtrValueParserParseAfter(PtrValueParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
ParseResult *CP_PtrValueParser_ParseResultGetExp(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (PtrValueToken *)p->token;
  return new ParseResult(t->exp);
}

#include "parser/ReturnParser.h"
ReturnParser *CompilerGetReturnParser(CC *cc) { return new ReturnParser(); }
ParseResult *CP_ReturnParserParse(ReturnParser *p, CC *cc, Source *s, int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_ReturnParserParseAfter(ReturnParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
ParseResult *CP_ReturnParser_ParseResultGetValue(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (ReturnToken *)p->token;
  if (!t->value)
    return 0;
  return new ParseResult(t->value);
}

#include "parser/SBlockParser.h"
SBlockParser *CompilerGetSBlockParser(CC *cc) { return new SBlockParser(); }
ParseResult *CP_SBlockParserParse(SBlockParser *p, CC *cc, Source *s, int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_SBlockParserParseAfter(SBlockParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
ParseResult *CP_SBlockParser_ParseResultGetStmt(ParseResult *p, int i) {
  if (!p)
    return 0;
  auto t = (SBlockToken *)p->token;
  if (i >= t->stmts.size())
    return 0;
  return new ParseResult(t->stmts[i]);
}

#include "parser/SizeofParser.h"
SizeofParser *CompilerGetSizeofParser(CC *cc) { return new SizeofParser(); }
ParseResult *CP_SizeofParserParse(SizeofParser *p, CC *cc, Source *s, int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_SizeofParserParseAfter(SizeofParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
ParseResult *CP_SizeofParser_ParseResultGetType(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (SizeofToken *)p->token;
  return new ParseResult(t->type);
}

#include "parser/StatementParser.h"
StatementParser *CompilerGetStatementParser(CC *cc) {
  return new StatementParser();
}
ParseResult *CP_StatementParserParse(StatementParser *p, CC *cc, Source *s,
                                     int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_StatementParserParseAfter(StatementParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}

#include "parser/StringParser.h"
StringParser *CompilerGetStringParser(CC *cc) { return new StringParser(); }
ParseResult *CP_StringParserParse(StringParser *p, CC *cc, Source *s, int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_StringParserParseAfter(StringParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
char *CP_StringParser_ParseResultGetValue(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (StringToken *)p->token;
  return STRTOCSTR(t->value);
}

#include "parser/StructParser.h"
StructParser *CompilerGetStructParser(CC *cc) { return new StructParser(); }
ParseResult *CP_StructParserParse(StructParser *p, CC *cc, Source *s, int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_StructParserParseAfter(StructParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
char *CP_StructParser_ParseResultGetId(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (StructToken *)p->token;
  return STRTOCSTR(t->id);
}
ParseResult *CP_StructParser_ParseResultGetMember(ParseResult *p, int i) {
  if (!p)
    return 0;
  auto t = (StructToken *)p->token;
  if (i >= t->members.size())
    return 0;
  return new ParseResult(t->members[i]);
}

#include "parser/SymbolParser.h"
SymbolParser *CompilerGetSymbolParser(CC *cc) { return new SymbolParser(); }
ParseResult *CP_SymbolParserParse(SymbolParser *p, CC *cc, Source *s, int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_SymbolParserParseAfter(SymbolParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
char *CP_SymbolParser_ParseResultGetSymbol(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (SymbolToken *)p->token;
  return STRTOCSTR(t->desc());
}

#include "parser/TopParser.h"
TopParser *CompilerGetTopParser(CC *cc) { return new TopParser(); }
ParseResult *CP_TopParserParse(TopParser *p, CC *cc, Source *s, int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_TopParserParseAfter(TopParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}

#include "parser/TypeParser.h"
TypeParser *CompilerGetTypeParser(CC *cc) { return new TypeParser(); }
ParseResult *CP_TypeParserParse(TypeParser *p, CC *cc, Source *s, int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_TypeParserParseAfter(TypeParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}

#include "parser/ValueParser.h"
ValueParser *CompilerGetValueParser(CC *cc) { return new ValueParser(); }
ParseResult *CP_ValueParserParse(ValueParser *p, CC *cc, Source *s, int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_ValueParserParseAfter(ValueParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}

#include "parser/VariableValueParser.h"
VariableValueParser *CompilerGetVariableValueParser(CC *cc) {
  return new VariableValueParser();
}
ParseResult *CP_VariableValueParserParse(VariableValueParser *p, CC *cc,
                                         Source *s, int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_VariableValueParserParseAfter(VariableValueParser *p,
                                              ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
char *CP_VariableValueParser_ParseResultGetValue(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (IdToken *)p->token;
  return STRTOCSTR(t->id);
}

#include "parser/WhileParser.h"
WhileParser *CompilerGetWhileParser(CC *cc) { return new WhileParser(); }
ParseResult *CP_WhileParserParse(WhileParser *p, CC *cc, Source *s, int pos) {
  auto ans = p->parse(cc, s, pos);
  return ans ? new ParseResult(ans) : 0;
}
ParseResult *CP_WhileParserParseAfter(WhileParser *p, ParseResult *pr) {
  if (!pr)
    return 0;
  auto ans = *pr >> *p;
  if (!ans)
    return 0;
  Token *a = ((TupleToken *)ans.token)->t2;
  return new ParseResult(a);
}
ParseResult *CP_WhileParser_ParseResultGetCond(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (WhileToken *)p->token;
  return new ParseResult(t->value);
}
ParseResult *CP_WhileParser_ParseResultGetStmt(ParseResult *p) {
  if (!p)
    return 0;
  auto t = (WhileToken *)p->token;
  return new ParseResult(t->statement);
}

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
      metaFunctions.insert(name);                                              \
    }                                                                          \
  }

void *CompileContext::getCompileObj(void *e) {

  llvm::ExecutionEngine *EE = (llvm::ExecutionEngine *)e;

  if (compileObj)
    return compileObj;

  compileObj = this; // nothign is insie, so we don't care

  // Initialize compile functions in the module.

  REGISTER_CALLBACK("Compiler$exit", "Compiler$exit_replace", &exitCallback);
  REGISTER_CALLBACK("Compiler$set", "Compiler$set_replace", &setCallback);
  REGISTER_CALLBACK("Compiler$get", "Compiler$get_replace", &getCallback);
  REGISTER_CALLBACK("Compiler$setFMeta", "Compiler$setFMeta_replace",
                    &CompilerSetFMeta);
  REGISTER_CALLBACK("Compiler$registerParser",
                    "Compiler$registerParser_replace", &registerParserCallback);

  REGISTER_CALLBACK("Compiler$createDynamicParseResult",
                    "Compiler$createDynamicParseResult_replace",
                    &CompilerCreateDynamicParseResult);

  REGISTER_CALLBACK("DynamicParseResult$set", "DynamicParseResult$set_replace",
                    &DynamicParseResultSet);
  REGISTER_CALLBACK("DynamicParseResult$get", "DynamicParseResult$get_replace",
                    &DynamicParseResultGet);

  REGISTER_CALLBACK("ParseResult$copyStart", "ParseResult$copyStart_replace",
                    &ParseResultCopyStart);
  REGISTER_CALLBACK("ParseResult$copyEnd", "ParseResult$copyEnd_replace",
                    &ParseResultCopyEnd);

  // Address
  REGISTER_CALLBACK("Compiler$createAddress", "Compiler$createAddress_replace",
                    &createAddress);
  REGISTER_CALLBACK("C_Address$getExp", "C_Address$getExp_replace",
                    &AddressGetExp);
  REGISTER_CALLBACK("C_Address$setExp", "C_Address$setExp_replace",
                    &AddressSetExp);

  // Arg
  REGISTER_CALLBACK("Compiler$createArg", "Compiler$createArg_replace",
                    &createArg);
  REGISTER_CALLBACK("C_Arg$getId", "C_Arg$getId_replace", &ArgGetId);
  REGISTER_CALLBACK("C_Arg$getType", "C_Arg$getType_replace", &ArgGetType);
  REGISTER_CALLBACK("C_Arg$setId", "C_Arg$setId_replace", &ArgSetId);
  REGISTER_CALLBACK("C_Arg$setType", "C_Arg$setType_replace", &ArgSetType);

  // Assign
  REGISTER_CALLBACK("Compiler$createAssign", "Compiler$createAssign_replace",
                    &createAssign);
  REGISTER_CALLBACK("C_Assign$getBase", "C_Assign$getBase_replace",
                    &AssignGetBase);
  REGISTER_CALLBACK("C_Assign$getExp", "C_Assign$getExp_replace",
                    &AssignGetExp);
  REGISTER_CALLBACK("C_Assign$setBase", "C_Assign$setBase_replace",
                    &AssignSetBase);
  REGISTER_CALLBACK("C_Assign$setExp", "C_Assign$setExp_replace",
                    &AssignSetExp);

  // BinOp
  REGISTER_CALLBACK("Compiler$createBinOp", "Compiler$createBinOp_replace",
                    &createBinOp);
  REGISTER_CALLBACK("C_BinOp$getLhs", "C_BinOp$getLhs_replace", &BinOpGetLHS);
  REGISTER_CALLBACK("C_BinOp$getRhs", "C_BinOp$getRhs_replace", &BinOpGetRHS);
  REGISTER_CALLBACK("C_BinOp$getOp", "C_BinOp$getOp_replace", &BinOpGetOp);
  REGISTER_CALLBACK("C_BinOp$setLHS", "C_BinOp$setLHS_replace", &BinOpSetLHS);
  REGISTER_CALLBACK("C_BinOp$setRHS", "C_BinOp$setRHS_replace", &BinOpSetRHS);
  REGISTER_CALLBACK("C_BinOp$setOp", "C_BinOp$setOp_replace", &BinOpSetOp);

  // Cast
  REGISTER_CALLBACK("Compiler$createCast", "Compiler$createCast_replace",
                    &createCast);
  REGISTER_CALLBACK("C_Cast$getType", "C_Cast$getType_replace", &CastGetType);
  REGISTER_CALLBACK("C_Cast$getExp", "C_Cast$getExp_replace", &CastGetExp);
  REGISTER_CALLBACK("C_Cast$setType", "C_Cast$setType_replace", &CastSetType);
  REGISTER_CALLBACK("C_Cast$setExp", "C_Cast$setExp_replace", &CastSetExp);

  // CodeBlock
  REGISTER_CALLBACK("Compiler$createCodeBlock",
                    "Compiler$createCodeBlock_replace", &createCodeBlock);
  REGISTER_CALLBACK("C_CodeBlock$addStmt", "C_CodeBlock$addStmt_replace",
                    &CodeBlockAddStmt);
  REGISTER_CALLBACK("C_CodeBlock$getStmt", "C_CodeBlock$getStmt_replace",
                    &CodeBlockGetStmt);

  // ConstBool
  REGISTER_CALLBACK("Compiler$createConstBool",
                    "Compiler$createConstBool_replace", &createConstBool);
  REGISTER_CALLBACK("C_ConstBool$get", "C_ConstBool$get_replace",
                    &ConstBoolGet);
  REGISTER_CALLBACK("C_ConstBool$set", "C_ConstBool$set_replace",
                    &ConstBoolSet);

  // ConstInt
  REGISTER_CALLBACK("Compiler$createConstInt",
                    "Compiler$createConstInt_replace", &createConstInt);
  REGISTER_CALLBACK("C_ConstInt$get", "C_ConstInt$get_replace", &ConstIntGet);
  REGISTER_CALLBACK("C_ConstInt$set", "C_ConstInt$set_replace", &ConstIntSet);

  // Define
  REGISTER_CALLBACK("Compiler$createDefine", "Compiler$createDefine_replace",
                    &createDefine);
  REGISTER_CALLBACK("C_Define$getId", "C_Define$getId_replace", &DefineGetId);
  REGISTER_CALLBACK("C_Define$getType", "C_Define$getType_replace",
                    &DefineGetType);
  REGISTER_CALLBACK("C_Define$getExp", "C_Define$getExp_replace",
                    &DefineGetExp);
  REGISTER_CALLBACK("C_Define$setId", "C_Define$setId_replace", &DefineSetId);
  REGISTER_CALLBACK("C_Define$setType", "C_Define$setType_replace",
                    &DefineSetType);
  REGISTER_CALLBACK("C_Define$setExp", "C_Define$setExp_replace",
                    &DefineSetExp);

  // Directive
  REGISTER_CALLBACK("Compiler$createDirective",
                    "Compiler$createDirective_replace", &createDirective);
  REGISTER_CALLBACK("C_Directive$getId", "C_Directive$getId_replace",
                    &DirectiveGetId);
  REGISTER_CALLBACK("C_Directive$getAST", "C_Directive$getAST_replace",
                    &DirectiveGetAST);
  REGISTER_CALLBACK("C_Directive$setId", "C_Directive$setId_replace",
                    &DirectiveSetId);
  REGISTER_CALLBACK("C_Directive$setAST", "C_Directive$setAST_replace",
                    &DirectiveSetAST);

  // FCall
  REGISTER_CALLBACK("Compiler$createFCall", "Compiler$createFCall_replace",
                    &createFCall);
  REGISTER_CALLBACK("C_FCall$getId", "C_FCall$getId_replace", &FCallGetId);
  REGISTER_CALLBACK("C_FCall$getArg", "C_FCall$getArg_replace", &FCallGetArg);
  REGISTER_CALLBACK("C_FCall$setId", "C_FCall$setId_replace", &FCallSetId);
  REGISTER_CALLBACK("C_FCall$addArg", "C_FCall$addArg_replace", &FCallAddArg);

  // FCallStmt
  REGISTER_CALLBACK("Compiler$createFCallStmt",
                    "Compiler$createFCallStmt_replace", &createFCallStmt);
  REGISTER_CALLBACK("C_FCallStmt$getId", "C_FCallStmt$getId_replace",
                    &FCallStmtGetId);
  REGISTER_CALLBACK("C_FCallStmt$getArg", "C_FCallStmt$getArg_replace",
                    &FCallStmtGetArg);
  REGISTER_CALLBACK("C_FCallStmt$setId", "C_FCallStmt$setId_replace",
                    &FCallStmtSetId);
  REGISTER_CALLBACK("C_FCallStmt$addArg", "C_FCallStmt$addArg_replace",
                    &FCallStmtAddArg);

  // Function
  REGISTER_CALLBACK("Compiler$createFunction",
                    "Compiler$createFunction_replace", &createFunction);
  REGISTER_CALLBACK("C_Function$getId", "C_Function$getId_replace",
                    &FunctionGetId);
  REGISTER_CALLBACK("C_Function$getReturntype",
                    "C_Function$getReturntype_replace", &FunctionGetReturntype);
  REGISTER_CALLBACK("C_Function$getStmt", "C_Function$getStmt_replace",
                    &FunctionGetStmt);
  REGISTER_CALLBACK("C_Function$getArg", "C_Function$getArg_replace",
                    &FunctionGetArg);
  REGISTER_CALLBACK("C_Function$setId", "C_Function$setId_replace",
                    &FunctionSetId);
  REGISTER_CALLBACK("C_Function$setReturntype",
                    "C_Function$setReturntype_replace", &FunctionSetReturnType);
  REGISTER_CALLBACK("C_Function$addStmt", "C_Function$addStmt_replace",
                    &FunctionAddStmt);
  REGISTER_CALLBACK("C_Function$addArg", "C_Function$addArg_replace",
                    &FunctionAddArg);

  // FunctionSig
  REGISTER_CALLBACK("Compiler$createFunctionSig",
                    "Compiler$createFunctionSig_replace", &createFunctionSig);
  REGISTER_CALLBACK("C_FunctionSig$getId", "C_FunctionSig$getId_replace",
                    &FunctionSigGetId);
  REGISTER_CALLBACK("C_FunctionSig$getReturntype",
                    "C_FunctionSig$getReturntype_replace",
                    &FunctionSigGetReturnType);
  REGISTER_CALLBACK("C_FunctionSig$getArg", "C_FunctionSig$getArg_replace",
                    &FunctionSigGetArg);
  REGISTER_CALLBACK("C_FunctionSig$isVararg", "C_FunctionSig$isVararg_replace",
                    &FunctionSigIsVararg);
  REGISTER_CALLBACK("C_FunctionSig$setId", "C_FunctionSig$setId_replace",
                    &FunctionSigSetId);
  REGISTER_CALLBACK("C_FunctionSig$setReturntype",
                    "C_FunctionSig$setReturntype_replace",
                    &FunctionSigSetReturnType);
  REGISTER_CALLBACK("C_FunctionSig$addArg", "C_FunctionSig$addArg_replace",
                    &FunctionSigAddArg);
  REGISTER_CALLBACK("C_FunctionSig$setVararg",
                    "C_FunctionSig$setVararg_replace", &FunctionSigSetVararg);

  // FunctionType
  REGISTER_CALLBACK("Compiler$createFunctionType",
                    "Compiler$createFunctionType_replace", &createFunctionType);
  REGISTER_CALLBACK("C_FunctionType$getReturntype",
                    "C_FunctionType$getReturntype_replace",
                    &FunctionTypeGetReturnType);
  REGISTER_CALLBACK("C_FunctionType$getArg", "C_FunctionType$getArg_replace",
                    &FunctionTypeGetArg);
  REGISTER_CALLBACK("C_FunctionType$isVararg",
                    "C_FunctionType$isVararg_replace", &FunctionTypeIsVararg);
  REGISTER_CALLBACK("C_FunctionType$setReturntype",
                    "C_FunctionType$setReturntype_replace",
                    &FunctionTypeSetReturnType);
  REGISTER_CALLBACK("C_FunctionType$addArg", "C_FunctionType$addArg_replace",
                    &FunctionTypeAddArg);
  REGISTER_CALLBACK("C_FunctionType$setVararg",
                    "C_FunctionType$setVararg_replace", &FunctionTypeSetVararg);

  // If
  REGISTER_CALLBACK("Compiler$createIf", "Compiler$createIf_replace",
                    &createIf);
  REGISTER_CALLBACK("C_If$getExp", "C_If$getExp_replace", &IfGetExp);
  REGISTER_CALLBACK("C_If$getSt1", "C_If$getSt1_replace", &IfGetSt1);
  REGISTER_CALLBACK("C_If$getSt2", "C_If$getSt2_replace", &IfGetSt2);
  REGISTER_CALLBACK("C_If$setExp", "C_If$setExp_replace", &IfSetExp);
  REGISTER_CALLBACK("C_If$setSt1", "C_If$setSt1_replace", &IfSetSt1);
  REGISTER_CALLBACK("C_If$setSt2", "C_If$setSt2_replace", &IfSetSt2);

  // Import
  REGISTER_CALLBACK("Compiler$createImport", "Compiler$createImport_replace",
                    &createImport);
  REGISTER_CALLBACK("C_Import$getPath", "C_Import$getPath_replace",
                    &ImportGetPath);
  REGISTER_CALLBACK("C_Import$setPath", "C_Import$setPath_replace",
                    &ImportSetPath);

  // Interface
  REGISTER_CALLBACK("Compiler$createInterface",
                    "Compiler$createInterface_replace", &createInterface);
  REGISTER_CALLBACK("C_Interface$getId", "C_Interface$getId_replace",
                    &InterfaceGetId);
  REGISTER_CALLBACK("C_Interface$getMethod", "C_Interface$getMethod_replace",
                    &InterfaceGetMethod);
  REGISTER_CALLBACK("C_Interface$setId", "C_Interface$setId_replace",
                    &InterfaceSetId);
  REGISTER_CALLBACK("C_Interface$setMethod", "C_Interface$setMethod_replace",
                    &InterfaceSetMethod);

  // MemAccess
  REGISTER_CALLBACK("Compiler$createMemAccess",
                    "Compiler$createMemAccess_replace", &createMemAccess);
  REGISTER_CALLBACK("C_MemAccess$getId", "C_MemAccess$getId_replace",
                    &MemAccessGetId);
  REGISTER_CALLBACK("C_MemAccess$getExp", "C_MemAccess$getExp_replace",
                    &MemAccessGetExp);
  REGISTER_CALLBACK("C_MemAccess$setId", "C_MemAccess$setId_replace",
                    &MemAccessSetId);
  REGISTER_CALLBACK("C_MemAccess$setExp", "C_MemAccess$setExp_replace",
                    &MemAccessSetExp);

  // Method
  REGISTER_CALLBACK("Compiler$createMethod", "Compiler$createMethod_replace",
                    &createMethod);
  REGISTER_CALLBACK("C_Method$getStructName", "C_Method$getStructName",
                    &MethodGetStructName);
  REGISTER_CALLBACK("C_Method$getMethodName", "C_Method$getMethodName",
                    &MethodGetMethodName);
  REGISTER_CALLBACK("C_Method$getFunction", "C_Method$getFunction",
                    &MethodGetFunction);
  REGISTER_CALLBACK("C_Method$setStructName", "C_Method$setStructName",
                    &MethodSetStructName);
  REGISTER_CALLBACK("C_Method$setMethodName", "C_Method$setMethodName",
                    &MethodSetMethodName);
  REGISTER_CALLBACK("C_Method$setFunction", "C_Method$setFunction",
                    &MethodSetFunction);

  // MethodCall
  REGISTER_CALLBACK("Compiler$createMethodCall",
                    "Compiler$createMethodCall_replace", &createMethodCall);
  REGISTER_CALLBACK("C_MethodCall$getExp", "C_MethodCall$getExp_replace",
                    &MethodCallGetExp);
  REGISTER_CALLBACK("C_MethodCall$setExp", "C_MethodCall$setExp_replace",
                    &MethodCallSetExp);
  REGISTER_CALLBACK("C_MethodCall$getArg", "C_MethodCall$getArg_replace",
                    &MethodCallGetArg);
  REGISTER_CALLBACK("C_MethodCall$addArg", "C_MethodCall$addArg_replace",
                    &MethodCallAddArg);

  // MethodCallStmt
  REGISTER_CALLBACK("Compiler$createMethodCallStmt",
                    "Compiler$createMethodCallStmt_replace",
                    &createMethodCallStmt);
  REGISTER_CALLBACK("C_MethodCallStmt$getExp",
                    "C_MethodCallStmt$getExp_replace", &MethodCallStmtGetExp);
  REGISTER_CALLBACK("C_MethodCallStmt$setExp",
                    "C_MethodCallStmt$setExp_replace", &MethodCallStmtSetExp);
  REGISTER_CALLBACK("C_MethodCallStmt$getArg",
                    "C_MethodCallStmt$getArg_replace", &MethodCallStmtGetArg);
  REGISTER_CALLBACK("C_MethodCallStmt$addArg",
                    "C_MethodCallStmt$addArg_replace", &MethodCallStmtAddArg);

  // NamedType
  REGISTER_CALLBACK("Compiler$createNamedType",
                    "Compiler$createNamedType_replace", &createNamedType);
  REGISTER_CALLBACK("C_NamedType$getId", "C_NamedType$getId_replace",
                    &NamedTypeGetId);
  REGISTER_CALLBACK("C_NamedType$setId", "C_NamedType$setId_replace",
                    &NamedTypeSetId);

  // PointerType
  REGISTER_CALLBACK("Compiler$createPointerType",
                    "Compiler$createPointerType_replace", &createPointerType);
  REGISTER_CALLBACK("C_PointerType$getType", "C_PointerType$getType_replace",
                    &PointerTypeGetType);
  REGISTER_CALLBACK("C_PointerType$setType", "C_PointerType$setType_replace",
                    &PointerTypeSetType);

  // PreOp
  REGISTER_CALLBACK("Compiler$createPreOp", "Compiler$createPreOp_replace",
                    &createPreOp);
  REGISTER_CALLBACK("C_PreOp$getOp", "C_PreOp$getOp_replace", &PreOpGetOp);
  REGISTER_CALLBACK("C_PreOp$getExp", "C_PreOp$getExp_replace", &PreOpGetExp);
  REGISTER_CALLBACK("C_PreOp$setOp", "C_PreOp$setOp_replace", &PreOpSetOp);
  REGISTER_CALLBACK("C_PreOp$setExp", "C_PreOp$setExp_replace", &PreOpSetExp);

  // PrimitiveType
  REGISTER_CALLBACK("Compiler$createPrimitiveType",
                    "Compiler$createPrimitiveType_replace",
                    &createPrimitiveType);
  REGISTER_CALLBACK("C_PrimitiveType$getKey", "C_PrimitiveType$getKey_replace",
                    &PrimitiveTypeGetKey);
  REGISTER_CALLBACK("C_PrimitiveType$setKey", "C_PrimitiveType$setKey_replace",
                    &PrimitiveTypeSetKey);

  // PtrValue
  REGISTER_CALLBACK("Compiler$createPtrValue",
                    "Compiler$createPtrValue_replace", &createPtrValue);
  REGISTER_CALLBACK("C_PtrValue$getPtr", "C_PtrValue$getPtr_replace",
                    &PtrValueGetPtr);
  REGISTER_CALLBACK("C_PtrValue$setPtr", "C_PtrValue$setPtr_replace",
                    &PtrValueSetPtr);

  // Return
  REGISTER_CALLBACK("Compiler$createReturn", "Compiler$createReturn_replace",
                    &createReturn);
  REGISTER_CALLBACK("C_Return$getExp", "C_Return$getExp_replace",
                    &ReturnGetExp);
  REGISTER_CALLBACK("C_Return$setExp", "C_Return$setExp_replace",
                    &ReturnSetExp);

  // Sizeof
  REGISTER_CALLBACK("Compiler$createSizeof", "Compiler$createSizeof_replace",
                    &createSizeof);
  REGISTER_CALLBACK("C_Sizeof$getType", "C_Sizeof$getType_replace",
                    &SizeofGetType);
  REGISTER_CALLBACK("C_Sizeof$setType", "C_Sizeof$setType_replace",
                    &SizeofSetType);

  // CString
  REGISTER_CALLBACK("Compiler$createCString", "Compiler$createCString_replace",
                    &createCString);
  REGISTER_CALLBACK("C_CString$getValue", "C_CString$getValue_replace",
                    &CStringGetValue);
  REGISTER_CALLBACK("C_CString$setValue", "C_CString$setValue_replace",
                    &CStringSetValue);

  // Struct
  REGISTER_CALLBACK("Compiler$createStruct", "Compiler$createStruct_replace",
                    &createStruct);
  REGISTER_CALLBACK("C_Struct$getId", "C_Struct$getId_replace", &StructGetId);
  REGISTER_CALLBACK("C_Struct$getMember", "C_Struct$getMember_replace",
                    &StructGetMember);
  REGISTER_CALLBACK("C_Struct$setId", "C_Struct$setId_replace", &StructSetId);
  REGISTER_CALLBACK("C_Struct$addMember", "C_Struct$addMember_replace",
                    &StructAddMember);
  REGISTER_CALLBACK("C_Struct$addMethod", "C_Struct$addMethod_replace",
                    &StructAddMethod);
  REGISTER_CALLBACK("Compiler$getStruct", "Compiler$getStruct_replace",
                    &CompilerGetStruct);

  // VariableValue
  REGISTER_CALLBACK("Compiler$createVariableValue",
                    "Compiler$createVariableValue_replace",
                    &createVariableValue);
  REGISTER_CALLBACK("C_VariableValue$getId", "C_VariableValue$getId_replace",
                    &VariableValueGetId);
  REGISTER_CALLBACK("C_VariableValue$setId", "C_VariableValue$setId_replace",
                    &VariableValueSetId);

  // While
  REGISTER_CALLBACK("Compiler$createWhile", "Compiler$createWhile_replace",
                    &createWhile);
  REGISTER_CALLBACK("C_While$getCond", "C_While$getCond_replace",
                    &WhileGetCond);
  REGISTER_CALLBACK("C_While$getStmt", "C_While$getStmt_replace",
                    &WhileGetStmt);
  REGISTER_CALLBACK("C_While$setCond", "C_While$setCond_replace",
                    &WhileSetCond);
  REGISTER_CALLBACK("C_While$setStmt", "C_While$setStmt_replace",
                    &WhileSetStmt);

  // ============== BASE AST
  REGISTER_CALLBACK("C_Type$toString", "C_Type$toString_replace",
                    &C_TypeToString);

  // ============== Parser
  REGISTER_CALLBACK("Compiler$getAddressParser",
                    "Compiler$getAddressParser_replace",
                    &CompilerGetAddressParser);
  REGISTER_CALLBACK("CP_AddressParser$parse", "CP_AddressParser$parse_replace",
                    &CP_AddressParserParse);
  REGISTER_CALLBACK("CP_AddressParser$parseAfter",
                    "CP_AddressParser$parseAfter_replace",
                    &CP_AddressParserParseAfter);
  REGISTER_CALLBACK("CP_AddressParser_ParseResult$getExp",
                    "CP_AddressParser_ParseResult$getExp_replace",
                    &CP_AddressParser_ParseResultGetExp);

  REGISTER_CALLBACK("Compiler$getAssignParser",
                    "Compiler$getAssignParser_replace",
                    &CompilerGetAssignParser);
  REGISTER_CALLBACK("CP_AssignParser$parse", "CP_AssignParser$parse_replace",
                    &CP_AssignParserParse);
  REGISTER_CALLBACK("CP_AssignParser$parseAfter",
                    "CP_AssignParser$parseAfter_replace",
                    &CP_AssignParserParseAfter);
  REGISTER_CALLBACK("CP_AssignParser_ParseResult$getExp",
                    "CP_AssignParser_ParseResult$getExp_replace",
                    &CP_AssignParser_ParseResultGetExp);
  REGISTER_CALLBACK("CP_AssignParser_ParseResult$getValue",
                    "CP_AssignParser_ParseResult$getValue_replace",
                    &CP_AssignParser_ParseResultGetValue);

  REGISTER_CALLBACK("Compiler$getBinOpParser",
                    "Compiler$getBinOpParser_replace", &CompilerGetBinOpParser);
  REGISTER_CALLBACK("CP_BinOpParser$parse", "CP_BinOpParser$parse_replace",
                    &CP_BinOpParserParse);
  REGISTER_CALLBACK("CP_BinOpParser$parseAfter",
                    "CP_BinOpParser$parseAfter_replace",
                    &CP_BinOpParserParseAfter);
  REGISTER_CALLBACK("CP_BinOpParser_ParseResult$getOp",
                    "CP_BinOpParser_ParseResult$getOp_replace",
                    &CP_BinOpParser_ParseResultGetOp);
  REGISTER_CALLBACK("CP_BinOpParser_ParseResult$getlhs",
                    "CP_BinOpParser_ParseResult$getlhs_replace",
                    &CP_BinOpParser_ParseResultGetlhs);
  REGISTER_CALLBACK("CP_BinOpParser_ParseResult$getrhs",
                    "CP_BinOpParser_ParseResult$getrhs_replace",
                    &CP_BinOpParser_ParseResultGetrhs);
  REGISTER_CALLBACK("CP_BinOpParser_ParseResult$getprec",
                    "CP_BinOpParser_ParseResult$getprec_replace",
                    &CP_BinOpParser_ParseResultGetprec);

  REGISTER_CALLBACK("Compiler$getBoolValueParser",
                    "Compiler$getBoolValueParser_replace",
                    &CompilerGetBoolValueParser);
  REGISTER_CALLBACK("CP_BoolValueParser$parse",
                    "CP_BoolValueParser$parse_replace",
                    &CP_BoolValueParserParse);
  REGISTER_CALLBACK("CP_BoolValueParser$parseAfter",
                    "CP_BoolValueParser$parseAfter_replace",
                    &CP_BoolValueParserParseAfter);
  REGISTER_CALLBACK("CP_BoolValueParser_ParseResult$getTruth",
                    "CP_BoolValueParser_ParseResult$getTruth_replace",
                    &CP_BoolValueParser_ParseResultGetTruth);

  REGISTER_CALLBACK("Compiler$getCastExprParser",
                    "Compiler$getCastExprParser_replace",
                    &CompilerGetCastExprParser);
  REGISTER_CALLBACK("CP_CastExprParser$parse",
                    "CP_CastExprParser$parse_replace", &CP_CastExprParserParse);
  REGISTER_CALLBACK("CP_CastExprParser$parseAfter",
                    "CP_CastExprParser$parseAfter_replace",
                    &CP_CastExprParserParseAfter);
  REGISTER_CALLBACK("CP_CastExprParser_ParseResult$getExp",
                    "CP_CastExprParser_ParseResult$getExp_replace",
                    &CP_CastExprParser_ParseResultGetExp);
  REGISTER_CALLBACK("CP_CastExprParser_ParseResult$getType",
                    "CP_CastExprParser_ParseResult$getType_replace",
                    &CP_CastExprParser_ParseResultGetType);

  REGISTER_CALLBACK("Compiler$getDefineParser",
                    "Compiler$getDefineParser_replace",
                    &CompilerGetDefineParser);
  REGISTER_CALLBACK("CP_DefineParser$parse", "CP_DefineParser$parse_replace",
                    &CP_DefineParserParse);
  REGISTER_CALLBACK("CP_DefineParser$parseAfter",
                    "CP_DefineParser$parseAfter_replace",
                    &CP_DefineParserParseAfter);
  REGISTER_CALLBACK("CP_DefineParser_ParseResult$getId",
                    "CP_DefineParser_ParseResult$getId_replace",
                    &CP_DefineParser_ParseResultGetId);
  REGISTER_CALLBACK("CP_DefineParser_ParseResult$getType",
                    "CP_DefineParser_ParseResult$getType_replace",
                    &CP_DefineParser_ParseResultGetType);
  REGISTER_CALLBACK("CP_DefineParser_ParseResult$getValue",
                    "CP_DefineParser_ParseResult$getValue_replace",
                    &CP_DefineParser_ParseResultGetValue);

  REGISTER_CALLBACK("Compiler$getDirectiveParser",
                    "Compiler$getDirectiveParser_replace",
                    &CompilerGetDirectiveParser);
  REGISTER_CALLBACK("CP_DirectiveParser$parse",
                    "CP_DirectiveParser$parse_replace",
                    &CP_DirectiveParserParse);
  REGISTER_CALLBACK("CP_DirectiveParser$parseAfter",
                    "CP_DirectiveParser$parseAfter_replace",
                    &CP_DirectiveParserParseAfter);
  REGISTER_CALLBACK("CP_DirectiveParser_ParseResult$getId",
                    "CP_DirectiveParser_ParseResult$getId_replace",
                    &CP_DirectiveParser_ParseResultGetId);
  REGISTER_CALLBACK("CP_DirectiveParser_ParseResult$getTop",
                    "CP_DirectiveParser_ParseResult$getTop_replace",
                    &CP_DirectiveParser_ParseResultGetTop);

  REGISTER_CALLBACK("Compiler$getExpressionParser",
                    "Compiler$getExpressionParser_replace",
                    &CompilerGetExpressionParser);
  REGISTER_CALLBACK("CP_ExpressionParser$parse",
                    "CP_ExpressionParser$parse_replace",
                    &CP_ExpressionParserParse);
  REGISTER_CALLBACK("CP_ExpressionParser$parseAfter",
                    "CP_ExpressionParser$parseAfter_replace",
                    &CP_ExpressionParserParseAfter);

  REGISTER_CALLBACK("Compiler$getFCallParser",
                    "Compiler$getFCallParser_replace", &CompilerGetFCallParser);
  REGISTER_CALLBACK("CP_FCallParser$parse", "CP_FCallParser$parse_replace",
                    &CP_FCallParserParse);
  REGISTER_CALLBACK("CP_FCallParser$parseAfter",
                    "CP_FCallParser$parseAfter_replace",
                    &CP_FCallParserParseAfter);
  REGISTER_CALLBACK("CP_FCallParser_ParseResult$getId",
                    "CP_FCallParser_ParseResult$getId_replace",
                    &CP_FCallParser_ParseResultGetId);
  REGISTER_CALLBACK("CP_FCallParser_ParseResult$getArg",
                    "CP_FCallParser_ParseResult$getArg_replace",
                    &CP_FCallParser_ParseResultGetArg);

  REGISTER_CALLBACK("Compiler$getFCallStmtParser",
                    "Compiler$getFCallStmtParser_replace",
                    &CompilerGetFCallStmtParser);
  REGISTER_CALLBACK("CP_FCallStmtParser$parse",
                    "CP_FCallStmtParser$parse_replace",
                    &CP_FCallStmtParserParse);
  REGISTER_CALLBACK("CP_FCallStmtParser$parseAfter",
                    "CP_FCallStmtParser$parseAfter_replace",
                    &CP_FCallStmtParserParseAfter);
  REGISTER_CALLBACK("CP_FCallStmtParser_ParseResult$getId",
                    "CP_FCallStmtParser_ParseResult$getId_replace",
                    &CP_FCallStmtParser_ParseResultGetId);
  REGISTER_CALLBACK("CP_FCallStmtParser_ParseResult$getArg",
                    "CP_FCallStmtParser_ParseResult$getArg_replace",
                    &CP_FCallStmtParser_ParseResultGetArg);

  REGISTER_CALLBACK("Compiler$getFunctionParser",
                    "Compiler$getFunctionParser_replace",
                    &CompilerGetFunctionParser);
  REGISTER_CALLBACK("CP_FunctionParser$parse",
                    "CP_FunctionParser$parse_replace", &CP_FunctionParserParse);
  REGISTER_CALLBACK("CP_FunctionParser$parseAfter",
                    "CP_FunctionParser$parseAfter_replace",
                    &CP_FunctionParserParseAfter);
  REGISTER_CALLBACK("CP_FunctionParser_ParseResult$getReturnType",
                    "CP_FunctionParser_ParseResult$getReturnType_replace",
                    &CP_FunctionParser_ParseResultGetReturnType);
  REGISTER_CALLBACK("CP_FunctionParser_ParseResult$getArg",
                    "CP_FunctionParser_ParseResult$getArg_replace",
                    &CP_FunctionParser_ParseResultGetArg);
  REGISTER_CALLBACK("CP_FunctionParser_ParseResult$getStmt",
                    "CP_FunctionParser_ParseResult$getStmt_replace",
                    &CP_FunctionParser_ParseResultGetStmt);

  REGISTER_CALLBACK("Compiler$getFunctionSigParser",
                    "Compiler$getFunctionSigParser_replace",
                    &CompilerGetFunctionSigParser);
  REGISTER_CALLBACK("CP_FunctionSigParser$parse",
                    "CP_FunctionSigParser$parse_replace",
                    &CP_FunctionSigParserParse);
  REGISTER_CALLBACK("CP_FunctionSigParser$parseAfter",
                    "CP_FunctionSigParser$parseAfter_replace",
                    &CP_FunctionSigParserParseAfter);
  REGISTER_CALLBACK("CP_FunctionSigParser_ParseResult$getReturnType",
                    "CP_FunctionSigParser_ParseResult$getReturnType_replace",
                    &CP_FunctionSigParser_ParseResultGetReturnType);
  REGISTER_CALLBACK("CP_FunctionSigParser_ParseResult$getArg",
                    "CP_FunctionSigParser_ParseResult$getArg_replace",
                    &CP_FunctionSigParser_ParseResultGetArg);

  REGISTER_CALLBACK("Compiler$getFunctionTypeParser",
                    "Compiler$getFunctionTypeParser_replace",
                    &CompilerGetFunctionTypeParser);
  REGISTER_CALLBACK("CP_FunctionTypeParser$parse",
                    "CP_FunctionTypeParser$parse_replace",
                    &CP_FunctionTypeParserParse);
  REGISTER_CALLBACK("CP_FunctionTypeParser$parseAfter",
                    "CP_FunctionTypeParser$parseAfter_replace",
                    &CP_FunctionTypeParserParseAfter);
  REGISTER_CALLBACK("CP_FunctionTypeParser_ParseResult$getReturnType",
                    "CP_FunctionTypeParser_ParseResult$getReturnType_replace",
                    &CP_FunctionTypeParser_ParseResultGetReturnType);
  REGISTER_CALLBACK("CP_FunctionTypeParser_ParseResult$getArg",
                    "CP_FunctionTypeParser_ParseResult$getArg_replace",
                    &CP_FunctionTypeParser_ParseResultGetArg);

  REGISTER_CALLBACK("Compiler$getIdParser", "Compiler$getIdParser_replace",
                    &CompilerGetIdParser);
  REGISTER_CALLBACK("CP_IdParser$parse", "CP_IdParser$parse_replace",
                    &CP_IdParserParse);
  REGISTER_CALLBACK("CP_IdParser$parseAfter", "CP_IdParser$parseAfter_replace",
                    &CP_IdParserParseAfter);
  REGISTER_CALLBACK("CP_IdParser_ParseResult$getId",
                    "CP_IdParser_ParseResult$getId_replace",
                    &CP_IdParser_ParseResultGetId);

  REGISTER_CALLBACK("Compiler$getIfParser", "Compiler$getIfParser_replace",
                    &CompilerGetIfParser);
  REGISTER_CALLBACK("CP_IfParser$parse", "CP_IfParser$parse_replace",
                    &CP_IfParserParse);
  REGISTER_CALLBACK("CP_IfParser$parseAfter", "CP_IfParser$parseAfter_replace",
                    &CP_IfParserParseAfter);
  REGISTER_CALLBACK("CP_IfParser_ParseResult$getValue",
                    "CP_IfParser_ParseResult$getValue_replace",
                    &CP_IfParser_ParseResultGetValue);
  REGISTER_CALLBACK("CP_IfParser_ParseResult$getSt1",
                    "CP_IfParser_ParseResult$getSt1_replace",
                    &CP_IfParser_ParseResultGetSt1);
  REGISTER_CALLBACK("CP_IfParser_ParseResult$getSt2",
                    "CP_IfParser_ParseResult$getSt2_replace",
                    &CP_IfParser_ParseResultGetSt2);

  REGISTER_CALLBACK("Compiler$getImportParser",
                    "Compiler$getImportParser_replace",
                    &CompilerGetImportParser);
  REGISTER_CALLBACK("CP_ImportParser$parse", "CP_ImportParser$parse_replace",
                    &CP_ImportParserParse);
  REGISTER_CALLBACK("CP_ImportParser$parseAfter",
                    "CP_ImportParser$parseAfter_replace",
                    &CP_ImportParserParseAfter);
  REGISTER_CALLBACK("CP_ImportParser_ParseResult$getPath",
                    "CP_ImportParser_ParseResult$getPath_replace",
                    &CP_ImportParser_ParseResultGetPath);

  REGISTER_CALLBACK("Compiler$getInterfaceParser",
                    "Compiler$getInterfaceParser_replace",
                    &CompilerGetInterfaceParser);
  REGISTER_CALLBACK("CP_InterfaceParser$parse",
                    "CP_InterfaceParser$parse_replace",
                    &CP_InterfaceParserParse);
  REGISTER_CALLBACK("CP_InterfaceParser$parseAfter",
                    "CP_InterfaceParser$parseAfter_replace",
                    &CP_InterfaceParserParseAfter);
  REGISTER_CALLBACK("CP_InterfaceParser_ParseResult$getId",
                    "CP_InterfaceParser_ParseResult$getId_replace",
                    &CP_InterfaceParser_ParseResultGetId);
  REGISTER_CALLBACK("CP_InterfaceParser_ParseResult$getMethod",
                    "CP_InterfaceParser_ParseResult$getMethod_replace",
                    &CP_InterfaceParser_ParseResultGetMethod);

  REGISTER_CALLBACK("Compiler$getKeywordParser",
                    "Compiler$getKeywordParser_replace",
                    &CompilerGetKeywordParser);
  REGISTER_CALLBACK("CP_KeywordParser$parse", "CP_KeywordParser$parse_replace",
                    &CP_KeywordParserParse);
  REGISTER_CALLBACK("CP_KeywordParser$parseAfter",
                    "CP_KeywordParser$parseAfter_replace",
                    &CP_KeywordParserParseAfter);
  REGISTER_CALLBACK("CP_KeywordParser_ParseResult$getKeyword",
                    "CP_KeywordParser_ParseResult$getKeyword_replace",
                    &CP_KeywordParser_ParseResultGetKeyword);

  REGISTER_CALLBACK("Compiler$getMemAccessParser",
                    "Compiler$getMemAccessParser_replace",
                    &CompilerGetMemAccessParser);
  REGISTER_CALLBACK("CP_MemAccessParser$parse",
                    "CP_MemAccessParser$parse_replace",
                    &CP_MemAccessParserParse);
  REGISTER_CALLBACK("CP_MemAccessParser$parseAfter",
                    "CP_MemAccessParser$parseAfter_replace",
                    &CP_MemAccessParserParseAfter);
  REGISTER_CALLBACK("CP_MemAccessParser_ParseResult$getId",
                    "CP_MemAccessParser_ParseResult$getId_replace",
                    &CP_MemAccessParser_ParseResultGetId);
  REGISTER_CALLBACK("CP_MemAccessParser_ParseResult$getExp",
                    "CP_MemAccessParser_ParseResult$getExp_replace",
                    &CP_MemAccessParser_ParseResultGetExp);

  REGISTER_CALLBACK("Compiler$getMethodCallParser",
                    "Compiler$getMethodCallParser_replace",
                    &CompilerGetMethodCallParser);
  REGISTER_CALLBACK("CP_MethodCallParser$parse",
                    "CP_MethodCallParser$parse_replace",
                    &CP_MethodCallParserParse);
  REGISTER_CALLBACK("CP_MethodCallParser$parseAfter",
                    "CP_MethodCallParser$parseAfter_replace",
                    &CP_MethodCallParserParseAfter);
  REGISTER_CALLBACK("CP_MethodCallParser_ParseResult$getExp",
                    "CP_MethodCallParser_ParseResult$getExp_replace",
                    &CP_MethodCallParser_ParseResultGetExp);
  REGISTER_CALLBACK("CP_MethodCallParser_ParseResult$getArg",
                    "CP_MethodCallParser_ParseResult$getArg_replace",
                    &CP_MethodCallParser_ParseResultGetArg);

  REGISTER_CALLBACK("Compiler$getMethodCallStmtParser",
                    "Compiler$getMethodCallStmtParser_replace",
                    &CompilerGetMethodCallStmtParser);
  REGISTER_CALLBACK("CP_MethodCallStmtParser$parse",
                    "CP_MethodCallStmtParser$parse_replace",
                    &CP_MethodCallStmtParserParse);
  REGISTER_CALLBACK("CP_MethodCallStmtParser$parseAfter",
                    "CP_MethodCallStmtParser$parseAfter_replace",
                    &CP_MethodCallStmtParserParseAfter);
  REGISTER_CALLBACK("CP_MethodCallStmtParser_ParseResult$getExp",
                    "CP_MethodCallStmtParser_ParseResult$getExp_replace",
                    &CP_MethodCallStmtParser_ParseResultGetExp);
  REGISTER_CALLBACK("CP_MethodCallStmtParser_ParseResult$getArg",
                    "CP_MethodCallStmtParser_ParseResult$getArg_replace",
                    &CP_MethodCallStmtParser_ParseResultGetArg);

  REGISTER_CALLBACK("Compiler$getMethodParser",
                    "Compiler$getMethodParser_replace",
                    &CompilerGetMethodParser);
  REGISTER_CALLBACK("CP_MethodParser$parse", "CP_MethodParser$parse_replace",
                    &CP_MethodParserParse);
  REGISTER_CALLBACK("CP_MethodParser$parseAfter",
                    "CP_MethodParser$parseAfter_replace",
                    &CP_MethodParserParseAfter);
  REGISTER_CALLBACK("CP_MethodParser_ParseResult$getStructName",
                    "CP_MethodParser_ParseResult$getStructName_replace",
                    &CP_MethodParser_ParseResultGetStructName);
  REGISTER_CALLBACK("CP_MethodParser_ParseResult$getMethodName",
                    "CP_MethodParser_ParseResult$getMethodName_replace",
                    &CP_MethodParser_ParseResultGetMethodName);
  REGISTER_CALLBACK("CP_MethodParser_ParseResult$getf",
                    "CP_MethodParser_ParseResult$getf_replace",
                    &CP_MethodParser_ParseResultGetf);

  REGISTER_CALLBACK("Compiler$getNamedTypeParser",
                    "Compiler$getNamedTypeParser_replace",
                    &CompilerGetNamedTypeParser);
  REGISTER_CALLBACK("CP_NamedTypeParser$parse",
                    "CP_NamedTypeParser$parse_replace",
                    &CP_NamedTypeParserParse);
  REGISTER_CALLBACK("CP_NamedTypeParser$parseAfter",
                    "CP_NamedTypeParser$parseAfter_replace",
                    &CP_NamedTypeParserParseAfter);
  REGISTER_CALLBACK("CP_NamedTypeParser_ParseResult$getId",
                    "CP_NamedTypeParser_ParseResult$getId_replace",
                    &CP_NamedTypeParser_ParseResultGetId);

  REGISTER_CALLBACK("Compiler$getNumberParser",
                    "Compiler$getNumberParser_replace",
                    &CompilerGetNumberParser);
  REGISTER_CALLBACK("CP_NumberParser$parse", "CP_NumberParser$parse_replace",
                    &CP_NumberParserParse);
  REGISTER_CALLBACK("CP_NumberParser$parseAfter",
                    "CP_NumberParser$parseAfter_replace",
                    &CP_NumberParserParseAfter);
  REGISTER_CALLBACK("CP_NumberParser_ParseResult$getValue",
                    "CP_NumberParser_ParseResult$getValue_replace",
                    &CP_NumberParser_ParseResultGetValue);

  REGISTER_CALLBACK("Compiler$getParenParser",
                    "Compiler$getParenParser_replace", &CompilerGetParenParser);
  REGISTER_CALLBACK("CP_ParenParser$parse", "CP_ParenParser$parse_replace",
                    &CP_ParenParserParse);
  REGISTER_CALLBACK("CP_ParenParser$parseAfter",
                    "CP_ParenParser$parseAfter_replace",
                    &CP_ParenParserParseAfter);
  REGISTER_CALLBACK("CP_ParenParser_ParseResult$getExp",
                    "CP_ParenParser_ParseResult$getExp_replace",
                    &CP_ParenParser_ParseResultGetExp);

  REGISTER_CALLBACK("Compiler$getPointerTypeParser",
                    "Compiler$getPointerTypeParser_replace",
                    &CompilerGetPointerTypeParser);
  REGISTER_CALLBACK("CP_PointerTypeParser$parse",
                    "CP_PointerTypeParser$parse_replace",
                    &CP_PointerTypeParserParse);
  REGISTER_CALLBACK("CP_PointerTypeParser$parseAfter",
                    "CP_PointerTypeParser$parseAfter_replace",
                    &CP_PointerTypeParserParseAfter);
  REGISTER_CALLBACK("CP_PointerTypeParser_ParseResult$getInnerType",
                    "CP_PointerTypeParser_ParseResult$getInnerType_replace",
                    &CP_PointerTypeParser_ParseResultGetInnerType);

  REGISTER_CALLBACK("Compiler$getPreOpParser",
                    "Compiler$getPreOpParser_replace", &CompilerGetPreOpParser);
  REGISTER_CALLBACK("CP_PreOpParser$parse", "CP_PreOpParser$parse_replace",
                    &CP_PreOpParserParse);
  REGISTER_CALLBACK("CP_PreOpParser$parseAfter",
                    "CP_PreOpParser$parseAfter_replace",
                    &CP_PreOpParserParseAfter);
  REGISTER_CALLBACK("CP_PreOpParser_ParseResult$getOp",
                    "CP_PreOpParser_ParseResult$getOp_replace",
                    &CP_PreOpParser_ParseResultGetOp);
  REGISTER_CALLBACK("CP_PreOpParser_ParseResult$getValue",
                    "CP_PreOpParser_ParseResult$getValue_replace",
                    &CP_PreOpParser_ParseResultGetValue);

  REGISTER_CALLBACK("Compiler$getPrimitiveTypeParser",
                    "Compiler$getPrimitiveTypeParser_replace",
                    &CompilerGetPrimitiveTypeParser);
  REGISTER_CALLBACK("CP_PrimitiveTypeParser$parse",
                    "CP_PrimitiveTypeParser$parse_replace",
                    &CP_PrimitiveTypeParserParse);
  REGISTER_CALLBACK("CP_PrimitiveTypeParser$parseAfter",
                    "CP_PrimitiveTypeParser$parseAfter_replace",
                    &CP_PrimitiveTypeParserParseAfter);
  REGISTER_CALLBACK("CP_PrimitiveTypeParser_ParseResult$getKey",
                    "CP_PrimitiveTypeParser_ParseResult$getKey_replace",
                    &CP_PrimitiveTypeParser_ParseResultGetKey);

  REGISTER_CALLBACK("Compiler$getPtrValueParser",
                    "Compiler$getPtrValueParser_replace",
                    &CompilerGetPtrValueParser);
  REGISTER_CALLBACK("CP_PtrValueParser$parse",
                    "CP_PtrValueParser$parse_replace", &CP_PtrValueParserParse);
  REGISTER_CALLBACK("CP_PtrValueParser$parseAfter",
                    "CP_PtrValueParser$parseAfter_replace",
                    &CP_PtrValueParserParseAfter);
  REGISTER_CALLBACK("CP_PtrValueParser_ParseResult$getExp",
                    "CP_PtrValueParser_ParseResult$getExp_replace",
                    &CP_PtrValueParser_ParseResultGetExp);

  REGISTER_CALLBACK("Compiler$getReturnParser",
                    "Compiler$getReturnParser_replace",
                    &CompilerGetReturnParser);
  REGISTER_CALLBACK("CP_ReturnParser$parse", "CP_ReturnParser$parse_replace",
                    &CP_ReturnParserParse);
  REGISTER_CALLBACK("CP_ReturnParser$parseAfter",
                    "CP_ReturnParser$parseAfter_replace",
                    &CP_ReturnParserParseAfter);
  REGISTER_CALLBACK("CP_ReturnParser_ParseResult$getValue",
                    "CP_ReturnParser_ParseResult$getValue_replace",
                    &CP_ReturnParser_ParseResultGetValue);

  REGISTER_CALLBACK("Compiler$getSBlockParser",
                    "Compiler$getSBlockParser_replace",
                    &CompilerGetSBlockParser);
  REGISTER_CALLBACK("CP_SBlockParser$parse", "CP_SBlockParser$parse_replace",
                    &CP_SBlockParserParse);
  REGISTER_CALLBACK("CP_SBlockParser$parseAfter",
                    "CP_SBlockParser$parseAfter_replace",
                    &CP_SBlockParserParseAfter);
  REGISTER_CALLBACK("CP_SBlockParser_ParseResult$getStmt",
                    "CP_SBlockParser_ParseResult$getStmt_replace",
                    &CP_SBlockParser_ParseResultGetStmt);

  REGISTER_CALLBACK("Compiler$getSizeofParser",
                    "Compiler$getSizeofParser_replace",
                    &CompilerGetSizeofParser);
  REGISTER_CALLBACK("CP_SizeofParser$parse", "CP_SizeofParser$parse_replace",
                    &CP_SizeofParserParse);
  REGISTER_CALLBACK("CP_SizeofParser$parseAfter",
                    "CP_SizeofParser$parseAfter_replace",
                    &CP_SizeofParserParseAfter);
  REGISTER_CALLBACK("CP_SizeofParser_ParseResult$getType",
                    "CP_SizeofParser_ParseResult$getType_replace",
                    &CP_SizeofParser_ParseResultGetType);

  REGISTER_CALLBACK("Compiler$getStatementParser",
                    "Compiler$getStatementParser_replace",
                    &CompilerGetStatementParser);
  REGISTER_CALLBACK("CP_StatementParser$parse",
                    "CP_StatementParser$parse_replace",
                    &CP_StatementParserParse);
  REGISTER_CALLBACK("CP_StatementParser$parseAfter",
                    "CP_StatementParser$parseAfter_replace",
                    &CP_StatementParserParseAfter);

  REGISTER_CALLBACK("Compiler$getStringParser",
                    "Compiler$getStringParser_replace",
                    &CompilerGetStringParser);
  REGISTER_CALLBACK("CP_StringParser$parse", "CP_StringParser$parse_replace",
                    &CP_StringParserParse);
  REGISTER_CALLBACK("CP_StringParser$parseAfter",
                    "CP_StringParser$parseAfter_replace",
                    &CP_StringParserParseAfter);
  REGISTER_CALLBACK("CP_StringParser_ParseResult$getValue",
                    "CP_StringParser_ParseResult$getValue_replace",
                    &CP_StringParser_ParseResultGetValue);

  REGISTER_CALLBACK("Compiler$getStructParser",
                    "Compiler$getStructParser_replace",
                    &CompilerGetStructParser);
  REGISTER_CALLBACK("CP_StructParser$parse", "CP_StructParser$parse_replace",
                    &CP_StructParserParse);
  REGISTER_CALLBACK("CP_StructParser$parseAfter",
                    "CP_StructParser$parseAfter_replace",
                    &CP_StructParserParseAfter);
  REGISTER_CALLBACK("CP_StructParser_ParseResult$getId",
                    "CP_StructParser_ParseResult$getId_replace",
                    &CP_StructParser_ParseResultGetId);
  REGISTER_CALLBACK("CP_StructParser_ParseResult$getMember",
                    "CP_StructParser_ParseResult$getMember_replace",
                    &CP_StructParser_ParseResultGetMember);

  REGISTER_CALLBACK("Compiler$getSymbolParser",
                    "Compiler$getSymbolParser_replace",
                    &CompilerGetSymbolParser);
  REGISTER_CALLBACK("CP_SymbolParser$parse", "CP_SymbolParser$parse_replace",
                    &CP_SymbolParserParse);
  REGISTER_CALLBACK("CP_SymbolParser$parseAfter",
                    "CP_SymbolParser$parseAfter_replace",
                    &CP_SymbolParserParseAfter);
  REGISTER_CALLBACK("CP_SymbolParser_ParseResult$getSymbol",
                    "CP_SymbolParser_ParseResult$getSymbol_replace",
                    &CP_SymbolParser_ParseResultGetSymbol);

  REGISTER_CALLBACK("Compiler$getTopParser", "Compiler$getTopParser_replace",
                    &CompilerGetTopParser);
  REGISTER_CALLBACK("CP_TopParser$parse", "CP_TopParser$parse_replace",
                    &CP_TopParserParse);
  REGISTER_CALLBACK("CP_TopParser$parseAfter",
                    "CP_TopParser$parseAfter_replace", &CP_TopParserParseAfter);

  REGISTER_CALLBACK("Compiler$getTypeParser", "Compiler$getTypeParser_replace",
                    &CompilerGetTypeParser);
  REGISTER_CALLBACK("CP_TypeParser$parse", "CP_TypeParser$parse_replace",
                    &CP_TypeParserParse);
  REGISTER_CALLBACK("CP_TypeParser$parseAfter",
                    "CP_TypeParser$parseAfter_replace",
                    &CP_TypeParserParseAfter);

  REGISTER_CALLBACK("Compiler$getValueParser",
                    "Compiler$getValueParser_replace", &CompilerGetValueParser);
  REGISTER_CALLBACK("CP_ValueParser$parse", "CP_ValueParser$parse_replace",
                    &CP_ValueParserParse);
  REGISTER_CALLBACK("CP_ValueParser$parseAfter",
                    "CP_ValueParser$parseAfter_replace",
                    &CP_ValueParserParseAfter);

  REGISTER_CALLBACK("Compiler$getVariableValueParser",
                    "Compiler$getVariableValueParser_replace",
                    &CompilerGetVariableValueParser);
  REGISTER_CALLBACK("CP_VariableValueParser$parse",
                    "CP_VariableValueParser$parse_replace",
                    &CP_VariableValueParserParse);
  REGISTER_CALLBACK("CP_VariableValueParser$parseAfter",
                    "CP_VariableValueParser$parseAfter_replace",
                    &CP_VariableValueParserParseAfter);
  REGISTER_CALLBACK("CP_VariableValueParser_ParseResult$getValue",
                    "CP_VariableValueParser_ParseResult$getValue_replace",
                    &CP_VariableValueParser_ParseResultGetValue);

  REGISTER_CALLBACK("Compiler$getWhileParser",
                    "Compiler$getWhileParser_replace", &CompilerGetWhileParser);
  REGISTER_CALLBACK("CP_WhileParser$parse", "CP_WhileParser$parse_replace",
                    &CP_WhileParserParse);
  REGISTER_CALLBACK("CP_WhileParser$parseAfter",
                    "CP_WhileParser$parseAfter_replace",
                    &CP_WhileParserParseAfter);
  REGISTER_CALLBACK("CP_WhileParser_ParseResult$getCond",
                    "CP_WhileParser_ParseResult$getCond_replace",
                    &CP_WhileParser_ParseResultGetCond);
  REGISTER_CALLBACK("CP_WhileParser_ParseResult$getStmt",
                    "CP_WhileParser_ParseResult$getStmt_replace",
                    &CP_WhileParser_ParseResultGetStmt);

  return compileObj;
}
