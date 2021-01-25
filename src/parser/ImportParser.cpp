#include "ImportParser.h"
#include "Keywords.h"
#include "parser.h"
#include "../ast/Import.h"

ImportToken::ImportToken(std::string importPath, CC *cc, Source *s, int spos, int epos)
    : importPath(importPath)
{
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

AST *ImportToken::toAST(CC *cc){
  return new Import(importPath);
}

std::string ImportToken::desc(){
  return "impot <" + importPath + ">";
}

ParseResult ImportParser::scheme(CC *cc, Source *s, int pos){
  auto ans = imp.parse(cc, s, pos) >> ip;
  if(!ans) return ans;

  std::string *id = &((IdToken*)((TupleToken*) ans.token)->t2)->id;

  return ParseResult(new ImportToken(*id, cc, s, ans.token->spos, ans.token->epos));
}

ImportParser::ImportParser()
  : imp(k_import)
{}
