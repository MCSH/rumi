#include "FCallParser.h"
#include "Symbols.h"

FCallToken::FCallToken(std::string id, CC *cc, Source *s, int spos, int epos)
  : id(id)
{
  this->spos = spos;
  this->epos = epos;
  this->cc = cc;
  this->s = s;
}

std::string FCallToken::desc(){
  std::string argsS = "";
  int len = args.size();
  if(len){
    argsS = " (";
    for (int i = 0; i < len; i++) {
      argsS += args[i]->desc();
      if (i != len - 1) {
        argsS += ", ";
      } else {
        argsS += ")";
      }
    }
  }
  return "<fcall " + id + argsS + ">";
}

ParseResult FCallParser::scheme(CC *cc, Source *s, int pos){
  // id ( )
  auto id = idp.parse(cc, s, pos) >> lpar;
  if(!id) return id;
  std::string *sid = &((IdToken*)((TupleToken*) id.token)->t1)->id;
  FCallToken *fct = new FCallToken(*sid, cc, s, id.token->spos, id.token->epos);
  // arguments
  auto tmp = id >> ep;
  if (!tmp){
    auto ans = id >> rpar; // no arguments
    fct->epos = ans.token->epos;
    return ParseResult(fct);
  }

  fct->args.push_back(((TupleToken*)tmp.token)->t2);

  // id ( v1,
  auto t = tmp >> cp;
  while(t){
    tmp = t >> ep;
    fct->args.push_back(((TupleToken*)tmp.token)->t2);
    t = tmp >> cp;
  }

  auto ans =  tmp >> rpar;
  if(!ans) return ans;
  fct->epos = ans.token->epos;
  return ParseResult(fct);
}

FCallParser::FCallParser()
  : lpar(s_lpar)
  , rpar(s_rpar)
  , cp(s_comma)
{}
