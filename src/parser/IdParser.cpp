#include "../Source.h"
#include "IdParser.h"
#include "../ast/VariableValue.h"

ParseResult IdParser::scheme(CC *cc, Source *s, int pos){
  if(kp.parse(cc, s, pos)) return ParseResult();
  pos = skipws(&s->str, pos);
  if(pos == -1) return ParseResult();
  int end = extractNextAlphaNumerical(&s->str, pos);
  if(end == -1) return ParseResult();
  std::string w = s->str.substr(pos, end - pos);
  return new IdToken(cc, s, pos, end - 1, w);
}

AST *IdToken::toAST(CC *cc){
  return new VariableValue(id);
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

IdToken::~IdToken(){
}
