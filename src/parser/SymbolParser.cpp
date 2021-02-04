#include "../Source.h"
#include "SymbolParser.h"
#include "Symbols.h"

ParseResult SymbolParser::scheme(CC *cc, Source *s, int pos){
  pos = skipws(&s->str, pos);
  if(pos == -1) return ParseResult();
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
  case '@':
    sb = s_at;
    break;
  case '~':
    sb = s_tilda;
    break;
  case '^':
    sb = s_caret;
    break;
  case '&':
    if(s->str.at(pos+1) == '&'){ // &&
      sb = s_andand;
      epos ++;
      break;
    }
    sb = s_and;
    break;
  case '|':
    if(s->str.at(pos+1) == '|'){ // ||
      sb = s_pipepipe;
      epos++;
      break;
    }
    sb = s_pipe;
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
  case '<':
    if(s->str.at(pos+1) == '='){ // <=
      sb = s_lte;
      epos ++;
      break;
    }
    if(s->str.at(pos+1) == '<'){ // <<
      sb = s_ltlt;
      epos ++;
      break;
    }
    sb = s_lt;
    break;
  case '>':
    if(s->str.at(pos+1) == '='){ // >=
      sb = s_gte;
      epos ++;
      break;
    }
    if(s->str.at(pos+1) == '>'){ // >>
      sb = s_gtgt;
      epos ++;
      break;
    }
    sb = s_gt;
    break;
  case '(':
    sb = s_lpar;
    break;
  case ')':
    sb = s_rpar;
    break;
  case '[':
    sb = s_lbra;
    break;
  case ']':
    sb = s_rbra;
    break;
  case '{':
    sb = s_lcbra;
    break;
  case '}':
    sb = s_rcbra;
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
    if(s->str.at(pos+1) == '='){ // !=
      sb = s_neq;
      epos ++;
      break;
    }
    sb = s_exc;
    break;
  default:
    return ParseResult();
  }

  if(sb_set && sb != this->sb) return ParseResult();
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
  case s_neq:
    return "!=";
  case s_eqeq:
    return "==";
  case s_and:
    return "&";
  case s_lpar:
    return "(";
  case s_rpar:
    return ")";
  case s_lbra:
    return "[";
  case s_rbra:
    return "]";
  case s_lcbra:
    return "{";
  case s_rcbra:
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
    return "...";
  case s_gt:
    return ">";
  case s_gte:
    return ">=";
  case s_gtgt:
    return ">>";
  case s_lt:
    return "<";
  case s_lte:
    return "<=";
  case s_ltlt:
    return "<<";
  case s_andand:
    return "&&";
  case s_pipe:
    return "|";
  case s_caret:
    return "^";
  case s_pipepipe:
    return "||";
  case s_at:
    return "@";
  case s_tilda:
    return "~";
  default:
    return "<Unprogrammed Symbol>";
  }
}

std::string SymbolToken::desc(){
  return "<Symbol " + symbolDesc(sb) + ">";
}
