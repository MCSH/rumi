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
    std::vector<Expression *> *arrE;
}

%token <string> ID
%token <string> DEC SSTRING 
                        
%token DEFINE_AND_ASSIGN
%token ARROW
%token RETURN
%token INT ANY STRING
%token TRIPLE_DOTS

%type <type> type

%type <exp> value expr variable function_call binary_operation

// TODO reorder these

%type <stmt> return_stmt stmt variable_decl variable_assign
%type <stmt> top_level function_define arg_decl vararg_decl

%type <functionSignature> function_signature 
%type <functionBody> function_body 

%type <arr> program top_levels stmts args_decl args_decl_list
%type <arrE> args args_list

%left '+' '-'
%left '*' '/' '%'

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
| function_signature ';' {$$=$1;}
;

function_define
: function_signature '{' function_body '}' {$$=new FunctionDefine($1, $3);}
;

function_signature
: ID DEFINE_AND_ASSIGN '(' args_decl_list ')' ARROW type {$$=new FunctionSignature($1, $4, $7);}
;

args_decl_list
: args_decl
| empty {$$=nullptr;}
;

empty:;

args_decl
: args_decl ',' arg_decl {$$=$1; $1->push_back($3);}
| args_decl ',' vararg_decl {$$=$1; $1->push_back($3);}
| arg_decl {$$=new std::vector<Statement *>(); $$->push_back($1);}
| vararg_decl {$$=new std::vector<Statement *>(); $$->push_back($1);}
;

vararg_decl
: ID ':' TRIPLE_DOTS type {$$=new ArgDecl($1, $4, true);}
;

arg_decl
: ID ':' type {$$=new ArgDecl($1, $3);}
;

function_body
: stmts {$$=new FunctionBody($1);/*TODO*/}
;

stmts
: stmts stmt {$$=$1; $$->push_back($2);}
| {$$=new std::vector<Statement *>();}
;

stmt
: return_stmt
| variable_decl
| variable_assign
| function_call ';' {$$=(Statement *)$1;}
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
| binary_operation
| '(' expr ')' {$$=$2;}
;

binary_operation
: expr '+' expr {$$=new BinaryOperation($1, Operation::PLUS, $3);}
| expr '-' expr {$$=new BinaryOperation($1, Operation::SUB, $3);}
| expr '*' expr {$$=new BinaryOperation($1, Operation::MULT, $3);}
| expr '/' expr {$$=new BinaryOperation($1, Operation::DIV, $3);}
| expr '%' expr {$$=new BinaryOperation($1, Operation::MOD, $3);}
;

function_call
: ID '(' args_list ')' {$$=new FunctionCallExpr($1, $3);}
;

args_list
: args
| empty {$$=nullptr;}
;

args
: args ',' expr {$$=$1; $1->push_back($3);}
| expr {$$=new std::vector<Expression *>(); $$->push_back($1);}
;

value
: DEC {$$=new IntValue($1);}
| SSTRING {$$=new StringValue($1);}
;

type
: INT {$$=new IntType();}
| STRING {$$=new StringType();}
| ANY {$$=new AnyType();}
;

%%

void yyerror(const char *s){
    extern int yylineno;	// defined and maintained in lex.c
    extern char *yytext;	// defined and maintained in lex.c
  
    printf("ERROR: %s at symbol %s on line %d\n", s, yytext, yylineno);

    exit(1);
}

void yyerror(std::string *s){
    yyerror(s->c_str());
}
