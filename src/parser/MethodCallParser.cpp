#include "MethodCallParser.h"
#include "MemAccessParser.h"
#include "Symbols.h"
#include "../ast/MethodCall.h"

MethodCallToken::MethodCallToken(Token *exp, CC *cc, Source *s, int spos,
                                 int epos)
  : exp(exp)
{
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

void *MethodCallToken::get(std::string key){
  if(key == "exp"){
    return exp;
  }
  // TODO args
  // TODO Error?
  return 0;
}

AST *MethodCallToken::toAST(CC *cc){
  MethodCall *m = new MethodCall();

  m->exp = (MemAccess *)exp->toAST(cc);
  for(Token *a: args){
    m->args.push_back((Expression *) a->toAST(cc));;
  }

  return m;
}
  
std::string MethodCallToken::desc(){
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
  return "<mcall " + exp->desc() + argsS + ">";
}

ParseResult MethodCallParser::scheme(CC *cc, Source *s, int pos){
  // id.id ... ( )
  auto id = map.parse(cc, s, pos);

  if(!id) return id;

  MethodCallToken *mct = new MethodCallToken(id.token, cc, s, pos, pos);

  id = id >> lpar;
  // arguments
  auto tmp = id >> ep;
  if (!tmp){
    auto ans = id >> rpar; // no arguments
    if(!ans) return ans;
    mct->epos = ans.token->epos;
    return ParseResult(mct);
  }

  mct->args.push_back(((TupleToken*)tmp.token)->t2);

  // id ( v1,
  auto t = tmp >> cp;
  while(t){
    tmp = t >> ep;
    mct->args.push_back(((TupleToken*)tmp.token)->t2);
    t = tmp >> cp;
  }

  auto ans =  tmp >> rpar;
  if(!ans) return ans;
  mct->epos = ans.token->epos;
  return ParseResult(mct);
}

MethodCallParser::MethodCallParser()
  : lpar(s_lpar)
  , rpar(s_rpar)
  , cp(s_comma)
{}
