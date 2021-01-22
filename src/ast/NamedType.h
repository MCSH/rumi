#pragma once
#include "Define.h"
#include "Type.h"
#include <vector>

class NamedType: public Type{
public:
  std::string id;

  Type *resolvedType = 0;

  NamedType(std::string id);

  virtual void set(std::string key, void *value) override;
  virtual void add(std::string key, void *value) override;

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

  virtual Type *baseType(CC *cc);

  virtual bool hasPreOp(CC *cc, std::string op) override;
  virtual Type *preoptyperesolve(CC *cc, std::string op) override;
  virtual void *preopgen(CC *cc, std::string op, Expression *value) override;
};
