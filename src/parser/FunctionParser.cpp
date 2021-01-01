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
  auto args = lpsp.parse(cc, s, pos); // (
  if(!(args >> rpsp)){
    do {
      args = args >> ip >> csp >> tp;
      if(!(args >> comsp)) break;
      args = args >> comsp;
    } while(args);
  }
  if(!args) return 0;
  auto sig =  args >> rpsp; // )
  auto a = sig >> asp >> tp; // -> type
  if(!a) a = sig;
  a = a >> lbsp;
  auto tmp = a >> sp;
  while(tmp){
    a = tmp;
    tmp = a >> sp;
  }
  return a >> rbsp;
}

FunctionParser::FunctionParser()
  : lpsp(s_lpar)
  , rpsp(s_rpar)
  , lbsp(s_lbra)
  , rbsp(s_rbra)
  , asp(s_to)
  , csp(s_col)
  , comsp(s_comma)
{
}
