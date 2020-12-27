#include "../base.h"
#include "SymbolParser.h"
#include "Symbols.h"

Token *SymbolParser::scheme(CC *cc, Source *s, int pos){
  pos = skipws(&s->str, pos);
  if(pos == -1) return 0;
  char c = s->str.at(pos);
  int epos = pos ;
  Symbol sb;
  switch(c){
  case '+':
    sb = s_plus;
    break;
  case ';':
    sb = s_semicolon;
    break;
  case ':':
    sb = s_col;
    break;
  case '=':
    sb = s_eq;
    break;
  case '(':
    sb = s_lpar;
    break;
  case ')':
    sb = s_rpar;
    break;
  case '{':
    sb = s_lbra;
    break;
  case '}':
    sb = s_rbra;
    break;
  case '-':
    c = s->str.at(pos+1);
    if(c == '>'){
      sb = s_to;
      epos++;
      break;
    }
    sb = s_minus;
    break;
  default:
    return 0;
  }

  if(sb_set && sb != this->sb) return 0;
  return new SymbolToken(sb, pos, epos, cc, s);
}

std::string SymbolToken::desc(){
  switch(sb){
  case s_plus:
    return "<Symbol +>";
  case s_semicolon:
    return "<Symbol ;>";
  case s_col:
    return "<Symbol :>";
  case s_eq:
    return "<Symbol =>";
  case s_lpar:
    return "<Symbol (>";
  case s_rpar:
    return "<Symbol )>";
  case s_lbra:
    return "<Symbol {>";
  case s_rbra:
    return "<Symbol }>";
  case s_to:
    return "<Symbol ->>";
  case s_minus:
    return "<Symbol ->";
  default:
    return "<Unprogrammed Symbol>";
  }
}
