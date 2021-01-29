#include "FCallStmtParser.h"
#include "Symbols.h"

FCallStmtToken::FCallStmtToken(std::string id, CC *cc, Source *s, int spos, int epos)
  : id(id)
{
  this->spos = spos;
  this->epos = epos;
  this->cc = cc;
  this->s = s;
}

FCallStmt *FCallStmtToken::toAST(CC *cc){
  FCallStmt *fs = new FCallStmt();
  FCall *f = &fs->fc;

  f->id = id;
  for(Token *a: args){
    f->args.push_back((Expression *)a->getAST(cc));
  }

  return fs;
}

std::string FCallStmtToken::desc(){
  std::string argsS = "";
  int len = args.size();
  if(len){
    argsS = " (";
    for (int i = 0; i < len; i++) {
      argsS += args[i]->desc();
      if (i != len - 1) {
        argsS += ", ";
      } else {
        argsS += ")";
      }
    }
  }
  return "<fcall " + id + argsS + ">";
}

ParseResult FCallStmtParser::scheme(CC *cc, Source *s, int pos){
  // id ( )
  auto id = idp.parse(cc, s, pos) >> lpar;
  if(!id) return id;
  std::string *sid = &((IdToken*)((TupleToken*) id.token)->t1)->id;
  FCallStmtToken *fct = new FCallStmtToken(*sid, cc, s, id.token->spos, id.token->epos);
  // arguments
  auto tmp = id >> ep;
  if (!tmp){
    auto ans = id >> rpar >> scp; // no arguments
    if(!ans) return ans;
    fct->epos = ans.token->epos;
    return ParseResult(fct);
  }

  fct->args.push_back(((TupleToken*)tmp.token)->t2);

  // id ( v1,
  auto t = tmp >> cp;
  while(t){
    tmp = t >> ep;
    fct->args.push_back(((TupleToken*)tmp.token)->t2);
    t = tmp >> cp;
  }

  auto ans =  tmp >> rpar >> scp;
  if(!ans) return ans;
  fct->epos = ans.token->epos;
  return ParseResult(fct);
}

FCallStmtParser::FCallStmtParser()
  : lpar(s_lpar)
  , rpar(s_rpar)
  , cp(s_comma)
  , scp(s_semicolon)
{}
