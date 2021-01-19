#include "../Source.h"
#include "SymbolParser.h"
#include "Symbols.h"

ParseResult SymbolParser::scheme(CC *cc, Source *s, int pos){
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
  case '&':
    sb = s_and;
    break;
  case ':':
    sb = s_col;
    break;
  case '=':
    if(s->str.at(pos+1) == '='){ // ==
      sb = s_eqeq;
      epos ++;
      break;
    }
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
  case '.':
    if( s->str.at(pos+1) == s->str.at(pos+2) ){
      if(s->str.at(pos+1) == '.'){
        sb = s_tripledot;
        epos += 2;
        break;
      }
    }
    sb = s_dot;
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
  case '*':
    sb = s_mult;
    break;
  case '/':
    sb = s_div;
    break;
  case '%':
    sb = s_rem;
    break;
  case ',':
    sb = s_comma;
    break;
  case '!':
    sb = s_exc;
    break;
  default:
    return 0;
  }

  if(sb_set && sb != this->sb) return 0;
  return new SymbolToken(sb, pos, epos, cc, s);
}

std::string symbolDesc(Symbol sb){
  switch(sb){
  case s_plus:
    return "+";
  case s_semicolon:
    return ";";
  case s_col:
    return ":";
  case s_eq:
    return "=";
  case s_eqeq:
    return "==";
  case s_and:
    return "&";
  case s_lpar:
    return "(";
  case s_rpar:
    return ")";
  case s_lbra:
    return "{";
  case s_rbra:
    return "}";
  case s_to:
    return "->";
  case s_minus:
    return "-";
  case s_mult:
    return "*";
  case s_div:
    return "/";
  case s_rem:
    return "%";
  case s_comma:
    return ",";
  case s_dot:
    return ".";
  case s_exc:
    return "!";
  case s_tripledot:
    return "<...>";
  default:
    return "<Unprogrammed Symbol>";
  }
}

std::string SymbolToken::desc(){
  return "<Symbol " + symbolDesc(sb) + ">";
}
