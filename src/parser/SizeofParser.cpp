#include "SizeofParser.h"
#include "Keywords.h"
#include "../ast/Sizeof.h"

SizeofToken::SizeofToken(Token *type, CC *cc, Source *s, int spos, int epos)
  : type(type)
{
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

void *SizeofToken::get(std::string key){
  if(key == "type"){
    return type;
  }
  // TODO Error?
  return 0;
}

std::string SizeofToken::desc(){
  return "sizeof " + type->desc();
}

AST *SizeofToken::toAST(CC *cc){
  return new Sizeof((Type*) type->toAST(cc));
}

SizeofParser::SizeofParser()
  : kp(k_sizeof)
{}

ParseResult SizeofParser::scheme(CC *cc, Source *s, int pos){
  auto ans = kp.parse(cc, s, pos) >> tp;
  if(!ans) return ans;

  Token *type = ((TupleToken*) ans.token)->t2;
  return ParseResult(new SizeofToken(type, cc, s, ans.token->spos, ans.token->epos));
}
