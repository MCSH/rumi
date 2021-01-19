#pragma once
#include "Define.h"
#include "Type.h"
#include <vector>
#include <map>

class FunctionSig;
class StructType;

class Interface: public Type{
public:
  std::string id;
  std::map<std::string, FunctionSig*> methods;
  std::set<std::string> structs;

  void *generatedType = 0;
  void *generatedTypePtr = 0;
  void *generatedVptr = 0;

  Interface(std::string id);

  void addMethod(CC *cc, FunctionSig *m);
  FunctionSig *resolveMethod(CC *cc, std::string methodId);
  int methodInd(CC *cc, std::string mid);

  virtual void* typegen(CC *cc);
  virtual void compile(CC *cc);
  virtual void prepare(CC *cc);

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

private:

  bool hasImplemented(StructType *st);
  void *resolveVptr(CC *cc, StructType *st);

};
