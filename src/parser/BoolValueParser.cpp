#include "BoolValueParser.h"
#include "Keywords.h"
#include "../ast/ConstBool.h"

BoolToken::BoolToken(CC *cc, Source *s, int spos, int epos, bool truth)
  : truth(truth)
{
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

std::string BoolToken::desc(){
  if(truth){
    return "true";
  } else {
    return "false";
  }
}

AST *BoolToken::toAST(CC *cc){
  return new ConstBool(truth);
}

ParseResult BoolValueParser::scheme(CC *cc, Source *s, int pos){
  auto t = tp.parse(cc, s, pos);
  if(t){
    BoolToken *bt = new BoolToken(cc, s, t.token->spos, t.token->epos, true);
    return ParseResult(bt);
  }

  auto f = fp.parse(cc, s, pos);
  if(f){
    BoolToken *bt = new BoolToken(cc, s, f.token->spos, f.token->epos, false);
    return ParseResult(bt);
  }

  return f;
}

BoolValueParser::BoolValueParser()
  : tp(k_true)
  , fp(k_false)
{}
