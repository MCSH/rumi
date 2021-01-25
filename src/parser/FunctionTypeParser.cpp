#include "FunctionTypeParser.h"
#include "Symbols.h"

FunctionTypeToken::FunctionTypeToken(CC *cc, Source *s, int pos, int epos){
  this->cc = cc;
  this->s = s;
  this->spos = pos;
  this->epos = epos;
}

std::string FunctionTypeToken::desc(){
  std::string ans = "(";
  int len = args.size();
  for(int i = 0; i < len; i++){
    ans += args[i] -> desc();
    if(i != len-1) ans += ", ";
  }
  ans += ") -> ";

  if(rt){
    ans += rt->desc();
  } else {
    ans += "unit";
  }

  return ans;
}

ParseResult FunctionTypeParser::scheme(CC *cc, Source *s, int pos){
  FunctionTypeToken *ftt = new FunctionTypeToken(cc, s, pos, 0);
  auto tmp = lparsp.parse(cc, s, pos); // (
  auto bak = tmp;
  while(true){
    tmp = tmp >> tp;
    if(!tmp) {
      tmp = bak;
      break;
    }

    ftt->args.push_back(((TupleToken*) tmp.token)->t2);
    
    if(!(tmp >> csp)) break;
    tmp = tmp >> csp;
  }

  auto ans = tmp >> rparsp >> tosp >> tp; // ) -> tp
  if(!ans) return ans;

  ftt->rt = ((TupleToken*) ans.token)->t2;

  ftt->spos = ans.token->spos;
  ftt->epos = ans.token->epos;
  return ParseResult(ftt);
}

FunctionTypeParser::FunctionTypeParser()
  : lparsp(s_lpar)
  , rparsp(s_rpar)
  , tosp(s_to)
  , csp(s_comma)
{}
