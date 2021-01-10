#include "DefineParser.h"
#include "../base.h"
#include "FunctionParser.h"
#include "FunctionSigParser.h"
#include "Symbols.h"
#include "../ast/Define.h"

DefineToken::DefineToken(std::string id, Token *type, Token *value, CC *CC, Source *s, int pos, int epos)
  : id(id)
  , type(type)
  , value(value)
{
  this->cc = cc;
  this->s = s;
  this->spos = pos;
  this->epos = epos;
}

std::string DefineToken::desc(){
  return "<Define "+ id + (type?" ("+type->desc()+")": "")+">" + (value?": " + value->desc(): "");
}

ParseResult DefineParser::scheme(CC *cc, Source *s, int pos){
  // TODO
  Token *type=0, *value=0;
  ParseResult id = ip.parse(cc, s, pos);
  if(!id) return ParseResult();
  std::string * sid = &((IdToken*) id.token) -> id;
  auto c = id >> csp; //  >> cc->parser.
  auto t = c >> tp;
  if(t)
    type = ((TupleToken*)t.token)->t2;
  if(t && t>> scp){
    auto ans = t >> scp; 
    if(!ans) return ans;
    return ParseResult(new DefineToken(*sid, type, value, cc, s, ans.token->spos, ans.token->epos));
  }
  if(!t) t = c;
  auto v = t >> esp >> vp;
  if(!v) return v;
  value = ((TupleToken*)v.token)->t2;
  return ParseResult(new DefineToken(*sid, type, value, cc, s, v.token->spos, v.token->epos));
}

DefineParser::DefineParser()
  :csp(s_col)
  ,scp(s_semicolon)
  ,esp(s_eq)
{}


AST *DefineToken::toAST(CC *cc){
  // TODO
  if(!type){
    if (FunctionBodyToken *fbt = dynamic_cast<FunctionBodyToken *>(value)) {
      // TODO We have a static function
      auto tmp = fbt->toAST(cc);

      tmp -> setId(id);
      return tmp;
    }
    if (FunctionSigToken *fst = dynamic_cast<FunctionSigToken *>(value)) {
      // TODO We have a static function
      auto tmp = fst->toAST(cc);

      tmp -> setId(id);
      return tmp;
    }
  }

  return new Define(id, value?(Expression *)value->toAST(cc):0, type?(Type *)type->toAST(cc):0);
}
