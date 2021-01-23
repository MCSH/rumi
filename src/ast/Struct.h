#pragma once
#include "Define.h"
#include "Type.h"
#include <vector>
#include <map>

class Method;
class Interface;

class StructType: public Type{
public:
  std::string id;
  std::vector<Define *> members;
  std::map<std::string, Method *> methods;

  void *generatedType = 0;

  StructType(std::string id);

  void addMethod(CC *cc, Method *m);

  Method *resolveMethod(CC *cc, std::string methodId);
  int resolveInterfaceVptrInd(CC *cc, Interface *it);

  virtual void* typegen(CC *cc) override;
  virtual void compile(CC *cc) override;
  virtual void prepare(CC *cc) override;

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
  int findmemindex(CC *cc, Expression *exp, std::string id);

  void resetBody(CC *cc);
};
