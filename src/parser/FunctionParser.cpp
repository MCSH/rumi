#include "FunctionParser.h"
#include "DefineParser.h"
#include <cassert>
#include <ostream>

#include "../base.h"
#include "../ast/Arg.h"

FunctionBodyToken::FunctionBodyToken(CC *cc, Source *s, int pos, int epos){
  this->cc = cc;
  this->s = s;
  this->spos = pos;
  this->epos = epos;
}

Function *FunctionBodyToken::toAST(CC *cc){
  Function *f = new Function();

  for(auto a: args){
    Arg *ar = new Arg();
    ar->id = a->id;
    ar->type = (Type *)a->type->getAST(cc);
    f->args.push_back(ar);
  }
  for(Token *t: statements){
    AST *a = t->getAST(cc);
    Statement *s = dynamic_cast<Statement *>(a);
    if(a && !s){
      graceFulExit(t->getDBG(), "Statement Token was not a real statement");
    }
    if(s) f->statements.push_back(s);
  }

  if(rt){
    f->returnType = (Type *)rt->getAST(cc);
  }

  return f;
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
      auto tip = args >> ip;
      args = tip >> csp >> tp;
      if(args){
        TupleToken *tt = (TupleToken*)args.token;
        Token *t = tt->t2;
        std::string *aid = &((IdToken*)((TupleToken*)tip.token)->t2)->id;
        fbt->args.push_back(new DefineToken(*aid, t, 0, cc, s, args.token->spos, args.token->epos));
      }
      if(!(args >> comsp)) break;
      args = args >> comsp;
    } while(args);
  }
  if(!args) {
    delete fbt;
    return ParseResult();
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
  , lbsp(s_lcbra)
  , rbsp(s_rcbra)
  , asp(s_to)
  , csp(s_col)
  , comsp(s_comma)
{
}
