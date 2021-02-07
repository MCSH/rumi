#include "../Source.h"
#include "NumberParser.h"
#include "../ast/ConstInt.h"
#include "../ast/ConstFloat.h"

ParseResult NumberParser::scheme(CC *cc, Source *s, int pos){
  int p = skipws(&s->str, pos);
  if(p == -1) return ParseResult();
  char n = s->str.at(p);
  if(!isnumerical(n)) return ParseResult();
  int val = 0;
  while(isnumerical(n)){
    val *= 10;
    val += n - '0';
    p++;
    n = s->str.at(p);
  }

  if(n == '.'){
    // We have a float
    p++;
    n = s->str.at(p);
    while (isnumerical(n)) {
      p++;
      n = s->str.at(p);
    }

    if(n == 'f'){
      double value = stod(s->str.substr(pos, p - pos));
      return new FloatNumberToken(cc, s, pos, p, value, true);
    }
    double value = stod(s->str.substr(pos, p - pos));
    return new FloatNumberToken(cc, s, pos, p-1, value, false);
  }

  auto ans = new NumberToken(cc, s, pos, p-1, val);
  return ans;
}

AST *NumberToken::toAST(CC *cc){
  return new ConstInt(val);
}

std::string NumberToken::desc(){
  return std::to_string(val);
}

NumberToken::NumberToken(CC *cc, Source *s, int spos, int epos, int val){
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
  this->val = val;
}

AST *FloatNumberToken::toAST(CC *cc){
  return new ConstFloat(val, is32);
}

std::string FloatNumberToken::desc(){
  return std::to_string(val);
}

FloatNumberToken::FloatNumberToken(CC *cc, Source *s, int spos, int epos, double val, bool is32){
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
  this->val = val;
  this->is32 = is32;
}
