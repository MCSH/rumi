#include "FunctionParser.h"

FunctionBodyToken::FunctionBodyToken(CC *cc, Source *s, int pos, int epos){
  this->cc = cc;
  this->s = s;
  this->spos = pos;
  this->epos = epos;
}

std::string FunctionBodyToken::desc(){
  std::string statementsS = "";
  std::string argsS = "";
  int len = args.size();
  if(len){
    argsS = "(";
    for(int i = 0; i < len; i++){
      argsS += args[i]->desc();
      if(i == len -1){
        argsS += ")";
      } else {
        argsS += ", ";
      }
    }
  }
  len = statements.size();
  if(len){
    for(int i = 0 ; i < len ; i++){
      statementsS += "\n   - " + statements[i]->desc();
    }
  }
  std::string rts = "";
  if(rt){
    rts = "\n-> " + rt->desc();
  }
  return "<Function "+argsS+">: " + statementsS + rts;
}

ParseResult FunctionParser::scheme(CC *cc, Source *s, int pos){
  FunctionBodyToken *fbt = new FunctionBodyToken(cc, s, pos, pos);
  auto args = lpsp.parse(cc, s, pos); // (
  if(!(args >> rpsp)){
    do {
      args = args >> ip >> csp >> tp;
      if(args){
        fbt->args.push_back(args.token);
      }
      if(!(args >> comsp)) break;
      args = args >> comsp;
    } while(args);
  }
  if(!args) {
    delete fbt;
    return 0; 
  }
  auto sig =  args >> rpsp; // )
  auto a = sig >> asp >> tp; // -> type
  if(a){
    fbt->rt = ((TupleToken*)a.token)->t2;
  } else a = sig;
  a = a >> lbsp;
  auto tmp = a >> sp;
  while(tmp){
    fbt->statements.push_back(((TupleToken*)tmp.token)->t2);
    a = tmp;
    tmp = a >> sp;
  }
  auto ans =  a >> rbsp;
  if(!ans) {
    delete fbt;
    return ans;
  }
  fbt->epos = ans.token->epos;
  return ParseResult(fbt);
}

FunctionParser::FunctionParser()
  : lpsp(s_lpar)
  , rpsp(s_rpar)
  , lbsp(s_lbra)
  , rbsp(s_rbra)
  , asp(s_to)
  , csp(s_col)
  , comsp(s_comma)
{
}
