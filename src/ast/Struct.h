#pragma once
#include "BinOpDef.h"
#include "Define.h"
#include "Function.h"
#include "Type.h"
#include <vector>
#include <map>

class Method;
class Interface;
class BinOpDef;

// TODO expose additions to compiler API
class StructType: public Type{
public:
  std::string id;
  std::vector<Define *> members;
  std::map<std::string, Method *> methods;
  std::map<std::string, BinOpDef *> binops;

  Function *initializer = 0;

  void *generatedType = 0;

  StructType(std::string id);

  void addMethod(CC *cc, Method *m);
  void addBinOp(CC *cc, BinOpDef *bod);

  Method *resolveMethod(CC *cc, std::string methodId);
  int resolveInterfaceVptrInd(CC *cc, Interface *it);

  virtual std::string toString() override;

  virtual void* typegen(CC *cc) override;
  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;

  virtual void initgen(CC *cc, Expression *alloca) override;

  virtual bool hasOp(CC *cc, std::string op, Expression *rhs) override;
  virtual void* opgen(CC *cc, Expression *lhs, std::string op, Expression *rhs) override;
  virtual Type* optyperesolve(CC *cc, std::string op, Expression *rhs) override;

  virtual Compatibility compatible(CC *cc, Type *t) override;
  virtual void* castgen(CC *cc, Expression *e) override;
  
  virtual bool hasMem(CC *cc, Expression *exp, std::string id) override;
  virtual void* memgen(CC *cc, Expression *exp, std::string id) override;
  virtual Type* memtyperesolve(CC *cc, Expression *exp, std::string id) override;
  virtual void* memalloca(CC *cc, Expression *exp, std::string id) override;

  virtual bool hasPreOp(CC *cc, std::string op) override;
  virtual Type *preoptyperesolve(CC *cc, std::string op) override;
  virtual void *preopgen(CC *cc, std::string op, Expression *value) override;

  virtual bool hasIndex(CC *cc, Expression *index) override;
  virtual void *indexgen(CC *cc, Expression *expr, Expression *index) override;
  virtual Type *indextyperesolve(CC *cc, Expression *index) override;
  virtual void *indexallocagen(CC *cc, Expression *expr, Expression *index) override;

private:
  int findmemindex(CC *cc, Expression *exp, std::string id);

  void resetBody(CC *cc);
};
