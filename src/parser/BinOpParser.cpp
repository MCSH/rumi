#include "BinOpParser.h"
#include "Symbols.h"
#include "../Source.h"
#include "../ast/BinOp.h"

BinOpToken::BinOpToken(Symbol op, Token *lhs, Token *rhs, CC *cc,
                       Source *s, int pos, int epos)
  : op(op)
  , lhs(lhs)
  , rhs(rhs)
    {
  this -> cc = cc;
  this -> s = s;
  this -> spos = pos;
  this -> epos = epos;
  // TODO handle the precedence
}

AST *BinOpToken::toAST(CC* cc){
  std::string ops = symbolDesc(op);
  return new BinOp((Expression *)lhs->toAST(cc), ops, (Expression *)rhs->toAST(cc));
}

std::string BinOpToken::desc(){
  return lhs -> desc() + " " + symbolDesc(op) + " " + rhs -> desc();
}

ParseResult BinOpParser::scheme(CC *cc, Source *s, int pos){
  ParseState *ps = s->resolveState(pos);
  if(ps->binOp) return ps->binToken; // recursive
  ps->binOp = true;

  Token* tmp =  innerscheme(cc, s, pos).token;

  // tmp ((lhs, op), rhs)

  if(tmp){
    TupleToken *t = (TupleToken*) tmp; 
    TupleToken *tt = (TupleToken *) t->t1;
    tmp = new BinOpToken(((SymbolToken *)tt->t2)->sb, tt->t1, t->t2, cc, s, t->spos, t->epos);
  }

  ps->binToken = tmp;
  return ParseResult(ps->binToken);
}

ParseResult BinOpParser::innerscheme(CC *cc, Source *s, int pos){
  // exp op exp
  auto v1 = vp.parse(cc, s, pos);

  // op = +?
  auto tmp = v1 >> addp;
  if(tmp) return tmp >> vp;

  // op = -?
  tmp = v1 >> minusp;
  if(tmp) return tmp >> vp;

  // op = *?
  tmp = v1 >> multp;
  if(tmp) return tmp >> vp;

  // op = /?
  tmp = v1 >> divp;
  if(tmp) return tmp >> vp;

  // op = %?
  tmp = v1 >> remp;
  if(tmp) return tmp >> vp;

  return 0;
}


BinOpParser::BinOpParser()
  : addp(s_plus)
  , minusp(s_minus)
  , multp(s_mult)
  , divp(s_div)
  , remp(s_rem)
{}
