#include "PrimitiveTypeParser.h"
#include "../Source.h"
#include "../CC.h"
#include "parser.h"

TypeToken::TypeToken(std::string key, int spos, int epos, CC *cc, Source *s)
  : key(key)
{
  this->spos = spos;
  this->epos = epos;
  this->cc = cc;
  this->s = s;
}

std::string TypeToken::desc(){
  return key;
}

ParseResult PrimitiveTypeParser::scheme(CC *cc, Source *s, int pos){
  int end = extractNextAlphaNumerical(&s->str, pos);
  if(end == -1) return 0;
  std::string w = s->str.substr(pos, end-pos);
  end--;
  if(w == "int")
    return new TypeToken(w, pos, end, cc, s);
  if(w == "f32")
    return new TypeToken(w, pos, end, cc, s);
  if(w == "f64")
    return new TypeToken(w, pos, end, cc, s);
  if(w == "any")
    return new TypeToken(w, pos, end, cc, s);
  if(w == "u8")
    return new TypeToken(w, pos, end, cc, s);
  if(w == "u16")
    return new TypeToken(w, pos, end, cc, s);
  if(w == "u32")
    return new TypeToken(w, pos, end, cc, s);
  if(w == "u64")
    return new TypeToken(w, pos, end, cc, s);
  if(w == "s8")
    return new TypeToken(w, pos, end, cc, s);
  if(w == "s16")
    return new TypeToken(w, pos, end, cc, s);
  if(w == "s32")
    return new TypeToken(w, pos, end, cc, s);
  if(w == "s64")
    return new TypeToken(w, pos, end, cc, s);
  if(w == "string")
    return new TypeToken(w, pos, end, cc, s);
  if(w == "unit")
    return new TypeToken(w, pos, end, cc, s);
  if(w == "void")
    return new TypeToken(w, pos, end, cc, s);
  return 0;
}
