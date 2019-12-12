%code requires { #include "node.h" }
%{
#include "node.h"
#include <string>

void yyerror(const char *s);
void yyerror(std::string *s);
extern int yylex();

std::vector<Statement *> *mainProgramNode;

%}

%union{
    std::string *string;
    //Node *node;
    Type *type;
    Expression *exp;
    Statement *stmt;
    FunctionSignature *functionSignature;
    FunctionBody *functionBody;
    std::vector<Statement *> *arr;
}

%token <string> ID
%token <string> DEC
                        
%token DEFINE_AND_ASSIGN
%token ARROW
%token RETURN
%token <type> INT

%type <type> type

%type <exp> value expr

%type <stmt> return_stmt stmt
%type <stmt> top_level function_define

%type <functionSignature> function_head 
%type <functionBody> function_body 

%type <arr> program top_levels stmts

%start program

%define parse.error verbose

%%

program
: top_levels {$$=$1; mainProgramNode = $$;}
;

top_levels
: top_level top_levels {$$=$2; $$->push_back($1);}
| {$$=new std::vector<Statement *>();}
;

top_level
: function_define {$$=$1;}
;

function_define
: function_head '{' function_body '}' {$$=new FunctionDefine($1, $3);}
;

function_head
: ID DEFINE_AND_ASSIGN '(' /*TODO*/ ')' ARROW type {$$=new FunctionSignature($1,$6);}
;

function_body
: stmts {$$=new FunctionBody($1);/*TODO*/}
;

stmts
: stmt stmts {$$=$2; $$->push_back($1);}
| {$$=new std::vector<Statement *>();}
;

stmt
: return_stmt {$$=$1;}
;

return_stmt
: RETURN expr ';' {$$=new ReturnStatement($2);}
;

expr
: value {$$=$1;}
;

value
: DEC {$$=new IntValue($1);}
;

type
: INT {$$=new IntType();}
;

%%

void yyerror(const char *s){
    extern int yylineno;	// defined and maintained in lex.c
    extern char *yytext;	// defined and maintained in lex.c
  
    printf("ERROR: %s at symbol %s on line %d\n", s, yytext, yylineno);
}

void yyerror(std::string *s){
    yyerror(s->c_str());
}
