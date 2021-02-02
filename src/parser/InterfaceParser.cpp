#include "InterfaceParser.h"
#include "FunctionSigParser.h"
#include "Keywords.h"
#include "Symbols.h"
#include "../ast/Interface.h"

InterfaceToken::InterfaceToken(std::string id, CC *cc, Source *s, int spos,
                               int eops)
  : id(id)
{
  this->cc = cc;
  this->s = s;
  this->spos = spos;
  this->epos = epos;
}

AST *InterfaceToken::toAST(CC *cc){
  // TODO
  Interface *in = new Interface(id);
  for(auto m: methods){
    in->addMethod(cc, (FunctionSig *)m->getAST(cc));
  }
  return in;
}

std::string InterfaceToken::desc(){
  std::string membersS = "";
  for(auto d: methods)
    membersS += "\n  * " + d->desc();
  return "interface " + id +": " + membersS;
}

ParseResult InterfaceParser::scheme(CC *cc, Source *s, int pos){
  auto id = idp.parse(cc, s, pos);
  auto tmp = id >> csp >> ikp >> lcsp;
  if(!tmp) return tmp;

  std::string ids = ((IdToken *) id.token) -> id;
  InterfaceToken *ift = new InterfaceToken(ids, cc, s, pos, pos);

  auto ans = tmp;

  tmp = ans >> dp;
  while(tmp){
    // ensure dp reterned a function signature
    DefineToken *t = (DefineToken*)((TupleToken *) tmp.token)->t2;
    if(!dynamic_cast<FunctionSigToken *>(t->value)){
      delete ift;
      return ParseResult();
    }

    ift->methods.push_back(t);
    
    ans = tmp;
    tmp = ans >> dp;
  }

  ans = ans >> rcsp;
  if(!ans){
    delete ift;
    return ans;
  }

  ift->spos = ans.token->spos;
  ift->epos = ans.token->epos;
  return ParseResult(ift);
}

InterfaceParser::InterfaceParser()
  : ikp(k_interface)
  , csp(s_col)
  , lcsp(s_lcbra)
  , rcsp(s_rcbra)
{}
