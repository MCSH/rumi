#include "MethodCallStmtParser.h"
#include "Symbols.h"
#include "../ast/MethodCall.h"

MethodCallStmtToken::MethodCallStmtToken(MethodCallToken *mct)
  : mct(mct)
{
  this->cc = mct->cc;
  this->s = mct->s;
  this->spos = mct->spos;
  this->epos = mct->epos;
}

std::string MethodCallStmtToken::desc(){
  return mct->desc();
}

AST *MethodCallStmtToken::toAST(CC *cc){
  auto a = (MethodCall *)mct->getAST(cc);
  return new MethodCallStmt(a);
}

ParseResult MethodCallStmtParser::scheme(CC *cc, Source *s, int pos){
  auto ans = mcp.parse(cc, s, pos) >> sp;

  if(!ans) return ans;

  auto token = (MethodCallToken *)((TupleToken*) ans.token)->t1;
  token->epos = ans.token->epos;
  return ParseResult(new MethodCallStmtToken(token));
}


MethodCallStmtParser::MethodCallStmtParser()
  : sp(s_semicolon)
{}
