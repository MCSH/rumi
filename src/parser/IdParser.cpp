#include "../base.h"
#include "IdParser.h"

Token *IdParser::scheme(CC *cc, Source *s, int pos){
  if(kp.scheme(cc, s, pos)) return 0;
  pos = skipws(&s->str, pos);
  if(pos == -1) return 0;
  int end = pos;
  int len = s->str.size();
  char c = s->str.at(pos);
  if(!isalpha(c)) return 0;
  do{
    end++;
    if(end >= len){
      break;
    }
    c = s->str.at(end);
  } while(isalphanumerical(c));
  std::string w = s->str.substr(pos, end - pos);
  return new IdToken(cc, s, pos, end, w);
}

IdToken::IdToken(CC *cc, Source *s, int spos, int epos, std::string val){
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
  this->id = val;
}

std::string IdToken::desc(){
  return "Id<" + id + ">";
}
