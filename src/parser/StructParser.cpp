#include "StructParser.h"
#include "Symbols.h"
#include "Keywords.h"
#include "../ast/Struct.h"

StructToken::StructToken(CC *cc, Source *s, int spos, int epos){
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

std::string StructToken::desc(){
  std::string membersS = "";
  for(auto d: members)
    membersS += "\n  * " + d->desc();
  return "struct " + id +": " + membersS;
}

AST *StructToken::toAST(CC *cc){
  auto ans =  new StructType(id);

  for(auto d: members){
    ans->members.push_back((Define *) d->toAST(cc));
  }
  
  return ans;
}

StructParser::StructParser()
  : lcsp(s_lbra)
  , rcsp(s_rbra)
  , csp(s_col)
  , skp(k_struct)
{}

#include "../base.h"

ParseResult StructParser::scheme(CC *cc, Source *s, int pos){
  StructToken *st = new StructToken(cc, s, pos, pos);
  auto base = idp.parse(cc, s, pos);

  if(!base) return base;
  st->id = ((IdToken *) base.token)->id;

  base = base >> csp >> skp >> lcsp;

  auto tmp = base >> dp;
  while(tmp){
    auto mem = (DefineToken *)((TupleToken *) tmp.token) -> t2;
    st->members.push_back(mem);
    base = tmp;
    tmp = base >> dp;
  }

  auto ans =  base >> rcsp;
  if(!ans){
    delete st;
    return ans;
  }
  st->spos = ans.token->spos;
  st->epos = ans.token->epos;
  return ParseResult(st);
}
