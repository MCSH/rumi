#pragma once
#include "KeywordParser.h"
#include "IdParser.h"

class ImportToken: public Token{
public:
  std::string importPath;

  ImportToken(std::string importPath, CC *cc, Source *s, int spos, int epos);

  virtual void *get(std::string key) override;
  virtual std::string desc() override;
  virtual AST *toAST(CC *cc) override;
};

class ImportParser: public ParseRule{
 public:
  virtual ParseResult scheme(CC *cc, Source *s, int pos) override;

  ImportParser();
private:
  KeywordParser imp;
  IdParser ip;
};
