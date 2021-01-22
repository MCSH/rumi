#include "CastExpr.h"
#include "Symbols.h"
#include "../Source.h"
#include "../ast/Cast.h"

CastToken::CastToken(Token *exp, Token *type, CC *cc, Source *s, int spos, int epos)
 : exp(exp)
 , type(type)
{
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

void *CastToken::get(std::string key){
  if(key == "exp"){
    return exp;
  }
  if(key == "type"){
    return type;
  }
  // TODO Error?
  return 0;
}

std::string CastToken::desc(){
  return exp->desc() + " -> " + type->desc();
}

AST *CastToken::toAST(CC *cc){
  return new Cast((Expression *)exp->toAST(cc), (Type *)type->toAST(cc));
}

ParseResult CastExpr::innerscheme(CC *cc, Source *s, int pos){
  auto v = exp.parse(cc, s, pos);
  auto ans = v >> asp >> tp;
  if(!ans) return ans;
  return ParseResult(new CastToken(v.token, ((TupleToken*) ans.token)->t2, cc, s, ans.token->spos, ans.token->epos));
}

ParseResult CastExpr::scheme(CC *cc, Source *s, int pos){
  ParseState *ps = s->resolveState(pos);
  if(ps->hasParser("cast")) return ps->getToken("cast"); // recursive
  ps->setToken("cast", 0);

  Token* tmp =  innerscheme(cc, s, pos).token;

  ps->setToken("cast", tmp);
  return ParseResult(tmp);
}


CastExpr::CastExpr()
  : asp(s_to)
{}
