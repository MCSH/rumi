#include "ReturnParser.h"
#include "Symbols.h"
#include "Keywords.h"
#include "../ast/Return.h"
#include "../base.h"
#include <ostream>

ReturnToken::ReturnToken(Token *value, CC *cc, Source *s, int spos, int epos)
  : value(value)
{
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

void *ReturnToken::get(std::string key){
  if(key == "value"){
    return value;
  }
  // TODO Error?
  return 0;
}

AST *ReturnToken::toAST(CC *cc){
  if(!value){
    return new Return(0);
  }

  Expression *e = dynamic_cast<Expression *>(value->toAST(cc));

  if(!e){
    cc->debug(NONE) << "Expected Expression didn't convert to Expression\n" << *value << std::endl;
    exit(1);
  }
  return new Return(e);
}

std::string ReturnToken::desc(){
  return "return" + (value?" " + value->desc(): "");
}

ParseResult ReturnParser::scheme(CC *cc, Source *s, int pos){
  auto r = rp.parse(cc, s, pos);
  auto tmp = r >> vp;
  if(tmp){
    // semicolon is handled inside that 
    Token *v = ((TupleToken*) tmp.token)->t2;
    return ParseResult(new ReturnToken(v, cc, s, tmp.token->spos, tmp.token->epos));
  }
  auto ans = r >> sp; // if no value, check for semicolon
  if(!ans) return ans;
  return ParseResult(new ReturnToken(0, cc, s, ans.token->spos, ans.token->epos));
}


ReturnParser::ReturnParser()
  : sp(s_semicolon)
  , rp(k_ret)
{}
