#pragma once

// TODO merge these two
class CodegenContext;
class CompileContext;

class Node{
public:
  int lineno;
  Node(){
    extern int yylineno;	// defined and maintained in lex.c
    lineno = yylineno;
  }

  virtual ~Node(){
  }

  virtual void compile(CompileContext *cc)=0;
};
