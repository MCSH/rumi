%{

#include <string>

void yyerror(const char *s);
void yyerror(std::string *s);
extern int yylex();

%}

%union{
    std::string *string;
 }

%start program

%define parse.error verbose

%%

program
:
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
