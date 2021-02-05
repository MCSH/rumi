#include "../Source.h"
#include "../CC.h"
#include "KeywordParser.h"
#include "Keywords.h"

KeywordToken* KeywordParser::findkey(CC *cc, Source *s, int pos){
  int end = extractNextAlphaNumerical(&s->str, pos);
  if(end == -1) return 0;
  std::string w = s->str.substr(pos, end - pos);
  end--;
  if(w == "return") return new KeywordToken(Keyword::k_ret, pos, end, cc, s);
  if(w == "if") return new KeywordToken(Keyword::k_if, pos, end, cc, s);
  if(w == "else") return new KeywordToken(Keyword::k_else, pos, end, cc, s);
  if(w == "while") return new KeywordToken(Keyword::k_while, pos, end, cc, s);
  if(w == "sizeof") return new KeywordToken(Keyword::k_sizeof, pos, end, cc, s);
  if(w == "struct") return new KeywordToken(Keyword::k_struct, pos, end, cc, s);
  if(w == "enum") return new KeywordToken(Keyword::k_enum, pos, end, cc, s);
  if(w == "interface") return new KeywordToken(Keyword::k_interface, pos, end, cc, s);
  if(w == "true") return new KeywordToken(Keyword::k_true, pos, end, cc, s);
  if(w == "false") return new KeywordToken(Keyword::k_false, pos, end, cc, s);
  if(w == "import") return new KeywordToken(Keyword::k_import, pos, end, cc, s);
  if(w == "new") return new KeywordToken(Keyword::k_new, pos, end, cc, s);
  if(w == "using") return new KeywordToken(Keyword::k_using, pos, end, cc, s);
  return 0;
}

ParseResult  KeywordParser::scheme(CC *cc, Source *s, int pos){
  auto t = findkey(cc, s, pos);
  if(!has_match) return t;
  if(!t) return ParseResult();
  if(t->kw == match) return t;
  delete t;
  return ParseResult();
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
  case k_sizeof:
    return "sizeof";
  case k_struct:
    return "struct";
  case k_enum:
    return "enum";
  case k_interface:
    return "interface";
  case k_true:
    return "true";
  case k_false:
    return "false";
  case k_import:
    return "import";
  case k_new:
    return "new";
  case k_using:
    return "using";
  default:
    return "[Unknown keyword]";
  }
}

KeywordParser::KeywordParser():has_match(false){}
KeywordParser::KeywordParser(Keyword kw):has_match(true),match(kw){}
