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
%token INT

%type <type> type

%type <exp> value expr variable function_call

// TODO reorder these

%type <stmt> return_stmt stmt variable_decl variable_assign
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
: top_levels top_level {$$=$1; $$->push_back($2);}
| {$$=new std::vector<Statement *>();}
;

top_level
: function_define
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
: stmts stmt {$$=$1; $$->push_back($2);}
| {$$=new std::vector<Statement *>();}
;

stmt
: return_stmt {$$=$1;}
| variable_decl {$$=$1;}
| variable_assign {$$=$1;}
;

variable_decl
: ID DEFINE_AND_ASSIGN expr ';' {$$=new VariableDecl($1, NULL, $3);}
| ID ':' type ';' {$$=new VariableDecl($1, $3);}
| ID ':' type '=' expr ';' {$$=new VariableDecl($1,$3,$5);}
;

variable_assign
: ID '=' expr ';' {$$=new VariableAssign($1, $3);}
;

return_stmt
: RETURN expr ';' {$$=new ReturnStatement($2);}
;

variable
: ID {$$=new VariableExpr($1);}
;

expr
: value
| function_call
| variable
;

function_call
: ID '(' ')' {$$=new FunctionCallExpr($1);}
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
