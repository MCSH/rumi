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

#include <iostream>

Token *FunctionParser::scheme(CC *cc, Source *s, int pos){
  auto sig =  lpsp.parse(cc, s, pos) >> rpsp;
  auto a = sig >> lbsp;
  auto tmp = a >> sp;
  while(tmp){
    a = tmp;
    tmp = a >> sp;
  }
  return a >> rbsp;
  //return id >> csp >> esp;
  //return id >> csp >> esp;
}

FunctionParser::FunctionParser()
  : lpsp(s_lpar)
  , rpsp(s_rpar)
  , lbsp(s_lbra)
  , rbsp(s_rbra)
{
}
