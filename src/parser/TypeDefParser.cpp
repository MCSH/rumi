#include "TypeDefParser.h"
#include "Keywords.h"
#include "../ast/TypeDef.h"

TypeDefToken::TypeDefToken(std::string id, Token *type, CC *cc, Source *s,
                           int pos, int epos)
  : id(id)
  , type(type)
{
  this->cc = cc;
  this->s = s;
  this->spos = pos;
  this->epos = epos;
}

std::string TypeDefToken::desc(){
  return "<Typedef " + id + " = " + type->desc() + ">";
}

AST *TypeDefToken::toAST(CC *cc){
  return new TypeDef(id, (Type *)type->getAST(cc));
}

TypeDefParser::TypeDefParser()
    : tkp(k_type), csp(s_col), scp(s_semicolon), esp(s_eq) {}

ParseResult TypeDefParser::scheme(CC *cc, Source *s, int pos){
  int spos = pos;

  auto base = idp.parse(cc, s, pos); // new_type
  if(!base) return base;
  
  auto type = base >> csp >> tkp >> esp >> tp ;//  : type = target_type
  
  auto ans = type >> scp; // ;
  if(!ans) return ans;

  std::string id = ((IdToken*) base.token) -> id;
  Token *typet = ((TupleToken *) type.token) -> t2;
  int epos = ans.token->epos;

  return ParseResult(new TypeDefToken(id, typet, cc, s, spos, epos));
}
