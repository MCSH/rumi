#include "FunctionSigParser.h"
#include "Symbols.h"
#include "../ast/Arg.h"

FunctionSigToken::FunctionSigToken(CC *cc, Source *s, int spos, int epos){
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

FunctionSig *FunctionSigToken::toAST(CC *cc){
  FunctionSig *fs = new FunctionSig();

  for(auto a: args){
    Arg *ar = new Arg();
    ar->id = a->id;
    ar->type = (Type *)a->type->toAST(cc);
    fs->args.push_back(ar);
  }

  fs->returnType = (Type *) returnType->toAST(cc);
  return fs;
}

std::string FunctionSigToken::desc(){
  std::string ans = "(";
  for(auto t: args) ans += t->desc() + ", ";
  return ans +")-> " + returnType->desc();
}

FunctionSigParser::FunctionSigParser()
  : lpsp(s_lpar)
  , rpsp(s_rpar)
  , asp(s_to)
  , ssp(s_semicolon)
  , csp(s_col)
  , comsp(s_comma)
{}

ParseResult FunctionSigParser::scheme(CC *cc, Source *s, int pos){
  FunctionSigToken *fst = new FunctionSigToken(cc, s, pos, pos);
  auto ans = lpsp.parse(cc, s, pos);
  if(!ans) return ans;
  auto tmp = ans >> ip >> csp >> tp;
  while(tmp){
    ans = tmp;
    auto firstTuple = (TupleToken*) tmp.token;
    auto secondTuple = (TupleToken*) firstTuple->t1;
    auto thirdTuple = (TupleToken*) secondTuple->t1;
    std::string id = ((IdToken*)((TupleToken*) thirdTuple)->t2)->id;
    Token *t = (firstTuple)->t2;
    fst->args.push_back(new DefineToken(id, t, 0, cc, s, ans.token->spos, ans.token->epos));
    if(!(tmp >> comsp)){
      break;
    }
    tmp = tmp >> comsp >> ip >> csp >> tp;
  }

  auto atp = ans >> rpsp >> asp >> tp;
  ans = atp >> ssp;
  if(ans){

    fst->returnType = ((TupleToken*) atp.token)->t2;
    
    fst->spos = ans.token->spos;
    fst->epos = ans.token->epos;
    return ParseResult(fst);
  }
  delete fst;
  return ans;
}
