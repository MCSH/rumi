#include "DirectiveParser.h"
#include "../base.h"
#include "Symbols.h"
#include "Token.h"
#include "../ast/Directive.h"

DirectiveToken::DirectiveToken(std::string id, Token *top, CC *cc, Source *s, int spos, int epos)
  : id(id)
  , top(top)
{
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

AST *DirectiveToken::toAST(CC *cc){
  return new Directive(id, top->toAST(cc));
}

std::string DirectiveToken::desc(){
  return "@" + id + " -- " + top->desc();
}

ParseResult DirectiveParser::scheme(CC *cc, Source *s, int pos){
  auto base = atp.parse(cc, s, pos) >> idp;
  if(!base) return base;

  auto top = cc->parser.parseTop(s, base.token->epos+1);
  if(!top) return top;

  std::string *id = &((IdToken *)((TupleToken*) base.token)->t2)->id;

  return ParseResult(new DirectiveToken(*id, top.token, cc, s, base.token->spos, top.token->epos));
}

DirectiveParser::DirectiveParser()
  : atp(s_at)
{}
