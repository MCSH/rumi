#include "FunctionParser.h"

FunctionToken::FunctionToken(CC *cc, Source *s, int pos, int epos){
  this->cc = cc;
  this->s = s;
  this->spos = pos;
  this->epos = epos;
}

std::string FunctionToken::desc(){
  return "<Function>";
}

Token *FunctionParser::scheme(CC *cc, Source *s, int pos){
  // TODO
  Token *id = ip.parse(cc, s, pos);
  // TODO id >> sp >> sp;
  //return -1;
  return id >> csp >> esp >> lpsp >> rpsp;
  //return id >> csp >> esp;
  //return id >> csp >> esp;
}

FunctionParser::FunctionParser()
  : csp(s_col)
  , esp(s_eq)
  , lpsp(s_lpar)
  , rpsp(s_rpar)
{
}
