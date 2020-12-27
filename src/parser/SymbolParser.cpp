#include "../base.h"
#include "SymbolParser.h"

Token *SymbolParser::scheme(CC *cc, Source *s, int pos){
  pos = skipws(&s->str, pos);
  if(pos == -1) return 0;
  char c = s->str.at(pos);
  int epos = pos + 1;
  Symbol sb;
  switch(c){
  case '+':
    sb = s_plus;
    break;
  case ';':
    sb = s_semicolon;
    break;
  default:
    return 0;
  }
  return new SymbolToken(sb, pos, epos, cc, s);
}

std::string SymbolToken::desc(){
  switch(sb){
  case s_plus:
    return "<Symbol +>";
  case s_semicolon:
    return "<Symbol ;>";
  default:
    return "<Unprogrammed Symbol>";
  }
}
