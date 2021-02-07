#include "StringParser.h"
#include "../Source.h"
#include "../ast/String.h"

StringToken::StringToken(std::string value, CC *cc, Source *s, int spos, int epos)
  : value(value)
{
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

AST *StringToken::toAST(CC *cc){
  return new String(value);
}

std::string StringToken::desc(){
  return '"' + value + '"';
}

ParseResult StringParser::scheme(CC *cc, Source *s, int pos){
  int p = skipws(&s->str, pos);
  if(p == -1) return ParseResult();

  char n = s->str.at(p);
  if(n != '"') return ParseResult();
  std::stringstream val;

  p++;
  n = s->str.at(p);
  while(n != '"'){

    if(n == '\\'){
      p++;
      n = s->str.at(p);
      switch (n) {
      case 'n':
        val << '\n';
        break;
      case 'r':
        val << '\r';
        break;
      default:
        val << n;
      }
    } else {
      val << n;
    }
    p++;
    n = s->str.at(p);
  }

  return new StringToken(val.str(), cc, s, pos, p);
}
