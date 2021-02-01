#pragma ocne
#include "parser.h"
#include "TypeParser.h"
#include "IdParser.h"
#include "OpParser.h"
#include "FunctionParser.h"

class BinOpDefToken: public Token{
public:
  std::string id;
  Token *rhs;
  std::string op;
  Token *f;
  BinOpDefToken(std::string id, std::string op, Token *rhs, Token *f, CC *cc, Source *s, int pos, int epos);

  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

// TODO Expose in Compiler API
class BinOpDefParser: public ParseRule{
public:
  ParseResult scheme(CC *cc, Source *s, int pos);
  BinOpDefParser();
private:
  IdParser idp;
  TypeParser tp;
  OpParser opp;
  FunctionParser fp;
  SymbolParser csp, eqp;
};
