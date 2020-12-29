#include "DefineParser.h"
#include "../base.h"

DefineToken::DefineToken(CC *CC, Source *s, int pos, int epos){
  this->cc = cc;
  this->s = s;
  this->spos = pos;
  this->epos = epos;
}

std::string DefineToken::desc(){
  return "<Define>";
}

Token *DefineParser::scheme(CC *cc, Source *s, int pos){
  Token *id = ip.parse(cc, s, pos);
  auto c = id >> csp; //  >> cc->parser.
  auto t = c >> tp;
  if(!t) t = c;
  auto v = t >> esp >> vp;
  return v;
  // TODO
}

DefineParser::DefineParser()
  :csp(s_col)
  ,esp(s_eq)
{}    
