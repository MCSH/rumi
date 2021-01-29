#include "CastExprParser.h"
#include "Symbols.h"
#include "../Source.h"
#include "../ast/Cast.h"

CastExprToken::CastExprToken(Token *exp, Token *type, CC *cc, Source *s, int spos, int epos)
 : exp(exp)
 , type(type)
{
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

std::string CastExprToken::desc(){
  return exp->desc() + " -> " + type->desc();
}

AST *CastExprToken::toAST(CC *cc){
  return new Cast((Expression *)exp->getAST(cc), (Type *)type->getAST(cc));
}

ParseResult CastExprParser::innerscheme(CC *cc, Source *s, int pos){
  auto v = exp.parse(cc, s, pos);
  auto ans = v >> asp >> tp;
  if(!ans) return ans;
  return ParseResult(new CastExprToken(v.token, ((TupleToken*) ans.token)->t2, cc, s, ans.token->spos, ans.token->epos));
}

ParseResult CastExprParser::scheme(CC *cc, Source *s, int pos){
  ParseState *ps = s->resolveState(pos);
  if(ps->hasParser("cast")) return ps->getToken("cast"); // recursive
  ps->setToken("cast", 0);

  Token* tmp =  innerscheme(cc, s, pos).token;

  ps->setToken("cast", tmp);
  return ParseResult(tmp);
}


CastExprParser::CastExprParser()
  : asp(s_to)
{}
