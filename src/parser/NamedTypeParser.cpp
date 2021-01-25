#include "NamedTypeParser.h"
#include "../ast/NamedType.h"

NamedTypeToken::NamedTypeToken(std::string typeId, CC *cc, Source *s, int spos, int epos)
  : typeId(typeId)
{
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

std::string NamedTypeToken::desc(){
  return typeId;
}

AST *NamedTypeToken::toAST(CC *cc){
  return new NamedType(typeId);
}

ParseResult NamedTypeParser::scheme(CC *cc, Source *s, int pos){
  auto ans = idp.parse(cc, s, pos);
  if(!ans) return ans;

  return ParseResult(new NamedTypeToken(((IdToken *) ans.token)->id, cc, s, ans.token->spos, ans.token->epos));
}
