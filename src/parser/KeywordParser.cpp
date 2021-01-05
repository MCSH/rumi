#include "../base.h"
#include "../CC.h"
#include "KeywordParser.h"
#include "Keywords.h"

KeywordToken* KeywordParser::findkey(CC *cc, Source *s, int pos){
  // TODO
  int end = extractNextAlphaNumerical(&s->str, pos);
  if(end == -1) return 0;
  std::string w = s->str.substr(pos, end - pos);
  end--;
  if(w == "return") return new KeywordToken(Keyword::k_ret, pos, end, cc, s);
  if(w == "if") return new KeywordToken(Keyword::k_if, pos, end, cc, s);
  if(w == "else") return new KeywordToken(Keyword::k_else, pos, end, cc, s);
  if(w == "while") return new KeywordToken(Keyword::k_while, pos, end, cc, s);
  return 0;
}

ParseResult  KeywordParser::scheme(CC *cc, Source *s, int pos){
  auto t = findkey(cc, s, pos);
  if(!has_match) return t;
  if(!t) return 0;
  if(t->kw == match) return t;
  delete t;
  return 0;
}

std::string KeywordToken::desc(){
  switch(kw){
  case k_ret:
    return "return";
  case k_if:
    return "if";
  case k_else:
    return "else";
  case k_while:
    return "while";
  default:
    return "[Unknown keyword]";
  }
}

KeywordParser::KeywordParser():has_match(false){}
KeywordParser::KeywordParser(Keyword kw):has_match(true),match(kw){}

/*
Token *TwoKeywordParser::scheme(CC *cc, Source *s, int pos){
  return kps.parse(cc, s, pos) >> kps;
}
*/
