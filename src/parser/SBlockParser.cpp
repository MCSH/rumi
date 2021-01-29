#include "SBlockParser.h"
#include "Symbols.h"
#include "Keywords.h"
#include "../ast/CodeBlock.h"

SBlockToken::SBlockToken(CC *cc, Source *s, int spos, int epos){
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

AST *SBlockToken::toAST(CC *cc){
  CodeBlock *cb = new CodeBlock();
  for(auto t: stmts){
    cb->stmts.push_back((Statement*) t->getAST(cc));
  }
  return cb;
}

std::string SBlockToken::desc(){
  std::string ans = "{";
  for(Token *t: stmts) ans += "-   " + t->desc() + "\n";
  return ans + "}";
}

ParseResult SBlockParser::scheme(CC *cc, Source *s, int pos){
  SBlockToken *sbt = new SBlockToken(cc, s, pos, pos);
  auto p = lbra.parse(cc, s, pos);
  auto tmp = p >> sp;
  while(tmp){
    sbt->stmts.push_back(((TupleToken *) tmp.token)->t2);
    p = tmp;
    tmp = p >> sp;
  }

  auto ans = p >> rbra;
  if(!ans){
    delete sbt;
    return ans;
  }

  sbt->spos = ans.token->spos;
  sbt->epos = ans.token->epos;

  return ParseResult(sbt);
}

SBlockParser::SBlockParser()
  : lbra(s_lbra)
  , rbra(s_rbra)
{}
