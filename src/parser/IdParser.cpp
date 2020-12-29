#include "../base.h"
#include "IdParser.h"

Token *IdParser::scheme(CC *cc, Source *s, int pos){
  if(kp.parse(cc, s, pos)) return 0;
  pos = skipws(&s->str, pos);
  if(pos == -1) return 0;
  int end = extractNextAlphaNumerical(&s->str, pos);
  std::string w = s->str.substr(pos, end - pos);
  return new IdToken(cc, s, pos, end - 1, w);
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
