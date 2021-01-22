#include "PreOpParser.h"
#include "Symbols.h"
#include "../ast/PreOp.h"

PreOpToken::PreOpToken(CC *cc, Source *s, int spos, int epos, std::string op, Token *value)
  : op(op)
  , value(value)
{
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

void *PreOpToken::get(std::string key){
  if(key == "op"){
    return (void *)op.c_str();
  }
  if(key == "value"){
    return value;
  }
  // TODO Error?
  return 0;
}

std::string PreOpToken::desc(){
  return op + value->desc();
}

AST *PreOpToken::toAST(CC *cc){
  return new PreOp(op, (Expression *)value->toAST(cc));
}

ParseResult PreOpParser::scheme(CC *cc, Source *s, int pos){
  auto e = excp.parse(cc, s, pos);

  if(!e) return e;

  auto v = e >> vp;

  if(!v) return v;

  Token * value = ((TupleToken*) v.token) -> t2;

  return ParseResult(new PreOpToken(cc, s, e.token->spos, v.token->epos, "!", value));
}

PreOpParser::PreOpParser()
  : excp(s_exc)
{}
