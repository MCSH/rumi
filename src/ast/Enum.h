#pragma once
#include "../parser/EnumParser.h"
#include "Type.h"
#include <map>

class Enum: public Type{
  int last;
  bool inited = false;
public:
  std::string id;
  std::map<std::string, EnumMember *> members;
  std::vector<std::string> memberOrders;

  Type *innerType;

  Enum();

  void addMember(CC *cc, EnumMember *m);
  void *enumMemGen(CC *cc, std::string id);

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
};
