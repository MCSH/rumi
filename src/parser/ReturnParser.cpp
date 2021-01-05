#include "ReturnParser.h"
#include "Symbols.h"
#include "Keywords.h"

ReturnToken::ReturnToken(Token *value, CC *cc, Source *s, int spos, int epos)
  : value(value)
{
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

std::string ReturnToken::desc(){
  return "return" + (value?" " + value->desc(): "");
}

ParseResult ReturnParser::scheme(CC *cc, Source *s, int pos){
  auto r = rp.parse(cc, s, pos);
  auto tmp = r >> vp;
  if(tmp){
    // semicolon is handled inside that 
    Token *v = ((TupleToken*) tmp.token)->t2;
    return ParseResult(new ReturnToken(v, cc, s, tmp.token->spos, tmp.token->epos));
  }
  auto ans = r >> sp; // if no value, check for semicolon
  if(!ans) return ans;
  return ParseResult(new ReturnToken(0, cc, s, ans.token->spos, ans.token->epos));
}


ReturnParser::ReturnParser()
  : sp(s_semicolon)
  , rp(k_ret)
{}
