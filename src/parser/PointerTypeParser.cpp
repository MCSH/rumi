#include "PointerTypeParser.h"
#include "Symbols.h"
#include "../ast/PointerType.h"

PointerTypeToken::PointerTypeToken(Token *innerType, CC *cc, Source *s, int pos,
                                   int epos)
  : innerType(innerType)
{
  this->cc = cc;
  this->s = s;
  this->spos = pos;
  this->epos = epos;
}

AST *PointerTypeToken::toAST(CC *cc){
  return new PointerType((Type*) innerType->toAST(cc));
}

std::string PointerTypeToken::desc(){
  return "* " + innerType->desc();
}

ParseResult PointerTypeParser::scheme(CC *cc, Source *s, int pos){
  auto ans = ssp.parse(cc, s, pos) >> tp;
  if(!ans) return ans;
  Token *innerType = ((TupleToken*) ans.token)->t2;
  return ParseResult(new PointerTypeToken(innerType, cc, s, ans.token->spos, ans.token->epos));
}

PointerTypeParser::PointerTypeParser()
  : ssp(s_mult)
{}
