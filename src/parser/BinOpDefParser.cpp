#include "BinOpDefParser.h"
#include "Symbols.h"
#include "../ast/BinOpDef.h"

BinOpDefToken::BinOpDefToken(std::string id, std::string op, Token *rhs,
                             Token *f, CC *cc, Source *s, int spos, int epos)
  : id(id)
  , op(op)
  , rhs(rhs)
  , f(f)
{
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

AST *BinOpDefToken::toAST(CC *cc){
  return new BinOpDef(id, op, (Type *)rhs->getAST(cc), (Function *)f->getAST(cc));
}

std::string BinOpDefToken::desc(){
  return id + " " + op + " " + rhs->desc() + ": " + f->desc();
}

ParseResult BinOpDefParser::scheme(CC *cc, Source *s, int pos){
  auto id = idp.parse(cc, s, pos);
  auto op = id >> opp;
  auto rhs = op >> tp;
  auto f = rhs >> csp >> eqp >> fp;

  if(!f) return f;
  std::string sop = symbolDesc(((SymbolToken *) ((TupleToken *) op.token)->t2)->sb);
  auto t = new BinOpDefToken(((IdToken *) id.token)->id, sop, ((TupleToken *) rhs.token)->t2, ((TupleToken *) f.token)->t2, cc, s, f.token->spos, f.token->epos);
  return ParseResult(t);
}

BinOpDefParser::BinOpDefParser()
  : csp(s_col)
  , eqp(s_eq)
{}
