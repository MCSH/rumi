#include "EnumParser.h"
#include "Symbols.h"
#include "Keywords.h"
#include <string>
#include "../ast/Enum.h"

EnumToken::EnumToken(CC *cc, Source *s, int spos, int epos){
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

std::string EnumToken::desc(){
  std::string vals = "";
  for(auto em: members){
    vals += "\n   * " + em->key;
    if(em->hasNum)
      vals += " = " + std::to_string(em->num);
  }
  return "enum " + id + ":" + vals;
}

AST *EnumToken::toAST(CC *cc){
  Enum *e = new Enum();
  e->setDBG(getDBG());
  e->id = id;
  for(auto em: members){
    e->addMember(cc, em);
  }
  return e;
}

ParseResult EnumParser::scheme(CC *cc, Source *s, int pos){
  EnumToken *et = new EnumToken(cc, s, pos, pos);
  auto base = idp.parse(cc, s, pos);
  if(!base) return base;

  et->id = ((IdToken *) base.token)->id;

  base = base >> csp >> ekp >> lcsp;

  auto tmp = base >> idp;
  while(tmp){
    EnumMember *em = new EnumMember();
    em->key = ((IdToken*)((TupleToken*) tmp.token)->t2)->id;
    if(tmp >> eqp >> np){
      tmp = tmp >> eqp >> np;
      em->hasNum = true;
      em->num = ((NumberToken *)((TupleToken*) tmp.token)->t2)->val;
    }
    // extract member
    et->members.push_back(em);

    base = tmp;
    tmp = tmp >> comsp >> idp;
  }

  if(base >> comsp) base = base >> comsp;
  auto ans = base >> rcsp;
  if(!ans){
    for(auto em: et->members){
      delete em;
    }
    delete et;
    return ans;
  }
  et->spos = ans.token->spos;
  et->epos = ans.token->epos;
  return ParseResult(et);
}

EnumParser::EnumParser()
  : ekp(k_enum)
  , csp(s_col)
  , comsp(s_comma)
  , eqp(s_eq)
  , lcsp(s_lbra)
  , rcsp(s_rbra)
{}
