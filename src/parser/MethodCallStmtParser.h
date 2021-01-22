#include "MethodCallParser.h"

class MethodCallStmtToken: public Token{
public:
  MethodCallToken *mct;

  MethodCallStmtToken(MethodCallToken *mct);

  virtual void *get(std::string key) override;
  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class MethodCallStmtParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos);

  MethodCallStmtParser();
private:
  MethodCallParser mcp;
  SymbolParser sp;
};
