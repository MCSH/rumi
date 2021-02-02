#include "IndexingParser.h"
#include "../Source.h"
#include "Symbols.h"
#include "../ast/Indexing.h"

IndexingToken::IndexingToken(Token *expr, Token *index, CC *cc, Source *s, int spos, int epos)
  : expr(expr)
  , index(index)
{
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

AST *IndexingToken::toAST(CC *cc){
  return new Indexing((Expression *)expr->getAST(cc), (Expression *)index->getAST(cc));
}

std::string IndexingToken::desc(){
  return expr->desc() + "[" + index->desc() + "]";
}

ParseResult IndexingParser::innerscheme(CC *cc, Source *s, int pos){
  auto e = vp.parse(cc, s, pos);

  auto ind = e >> lbra >> ip;
  auto ans = ind >> rbra;
  
  if(!ans) return ans;
  return new IndexingToken(e.token, ((TupleToken *) ind.token)->t2, cc, s, ans.token->spos, ans.token->epos);
}

ParseResult IndexingParser::scheme(CC *cc, Source *s, int pos){
  ParseState *ps = s->resolveState(pos);
  if(ps->hasParser("indexing")) return ps->getToken("indexing"); // recursive
  ps->setToken("indexing", 0);

  ParseResult tmp =  innerscheme(cc, s, pos).token;

  if(tmp)
    ps->setToken("indexing", tmp.token);
  return tmp;
}

int IndexingParser::prec() {
  return 10; // TODO check this
}

IndexingParser::IndexingParser()
  : lbra(s_lbra)
  , rbra(s_rbra)
{}
