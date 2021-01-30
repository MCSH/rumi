#include "NewParser.h"
#include "Symbols.h"
#include "Keywords.h"
#include "../ast/Method.h"

ParseResult NewParser::scheme(CC *cc, Source *s, int pos){
  auto tmp = kw.parse(cc, s, pos) >> idp;
  auto ans = tmp >> csp >> esp >> fp;
  if(!ans) return ans;

  std::string sid = ((IdToken *)((TupleToken *) tmp.token)->t2)->id;
  Token *f = ((TupleToken*) ans.token) -> t2;

  return ParseResult(new MethodToken(sid, "!new", f, cc, s, ans.token->spos, ans.token->epos));
}

NewParser::NewParser()
  : csp(s_col)
  , esp(s_eq)
  , kw(k_new)
{}
