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
    ar->type = (Type *)a->type->getAST(cc);
    fs->args.push_back(ar);
  }

  fs->returnType = (Type *) returnType->getAST(cc);
  fs->vararg = vararg;
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
  , tripledotsp(s_tripledot)
{}

ParseResult FunctionSigParser::scheme(CC *cc, Source *s, int pos){
  FunctionSigToken *fst = new FunctionSigToken(cc, s, pos, pos);
  auto ans = lpsp.parse(cc, s, pos);
  if(!ans) return ans;
  auto tmpId = ans >> ip;
  auto tmpC = tmpId >> csp;
  auto tmp3 = tmpC >> tripledotsp;
  if(tmp3){
    fst->vararg = true;
    tmpC = tmp3;
  }
  auto tmp = tmpC >> tp;
  while(tmp){
    ans = tmp;
    std::string id = ((IdToken*)((TupleToken*) tmpId.token)->t2)->id;
    Token *t = ((TupleToken*) tmp.token)->t2;
    fst->args.push_back(new DefineToken(id, t, 0, cc, s, ans.token->spos, ans.token->epos));
    if(!(tmp >> comsp)){
      break;
    }
    tmpId = tmp >> comsp >> ip;
    tmpC = tmpId >> csp;
    tmp3 = tmpC >> tripledotsp;
    if (tmp3) {
      fst->vararg = true;
      tmpC = tmp3;
    }
    tmp = tmpC >> tp;
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
