#pragma once
#include "Define.h"
#include "Type.h"
#include <vector>
#include <map>

class Method;

class StructType: public Type{
public:
  std::string id;
  std::vector<Define *> members;
  std::map<std::string, Method *> methods;

  void *generatedType = 0;

  StructType(std::string id);

  void addMethod(CC *cc, Method *m);

  virtual void* typegen(CC *cc);
  virtual void compile(CC *cc);
  virtual void prepare(CC *cc);

  virtual bool hasOp(CC *cc, std::string op, Expression *rhs) override;
  virtual void* opgen(CC *cc, Expression *lhs, std::string op, Expression *rhs) override;
  virtual Type* optyperesolve(CC *cc, std::string op, Expression *rhs) override;

  virtual Compatibility compatible(Type *t) override;
  virtual void* castgen(CC *cc, Expression *e) override;
  
  virtual bool hasMem(CC *cc, Expression *exp, std::string id) override;
  virtual void* memgen(CC *cc, Expression *exp, std::string id) override;
  virtual Type* memtyperesolve(CC *cc, Expression *exp, std::string id) override;
  virtual void* memalloca(CC *cc, Expression *exp, std::string id) override;

private:
  int findmemindex(CC *cc, Expression *exp, std::string id);
};
