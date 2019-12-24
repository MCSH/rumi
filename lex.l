WHITESPACE [ \t\v\n\r\f]
ALPHA [a-zA-Z_]
ALPHANUMERIC [a-zA-Z_0-9]
DIGIT [0-9]
NONZERODIGIT [1-9]

%{
#include <string>
#include "parse.hpp"
  // C Code goes here
int comment_nesting = 0;
%}

%x COMMENT
%x LINE_COMMENT
%option yylineno

%%

  /* Symbols */
":=" {return DEFINE_AND_ASSIGN;}
"->" {return ARROW;}
"..." {return TRIPLE_DOTS;}
"."{WHITESPACE}+"as" {return AS;}

"(" {return '(';}
")" {return ')';}
"{" {return '{';}
"}" {return '}';}
";" {return ';';}
":" {return ':';}
"=" {return '=';}
"," {return ',';}
"[" {return '[';}
"]" {return ']';}

"*" {return '*';}
"+" {return '+';}
"-" {return '-';}
"/" {return '/';}
"%" {return '%';}
"." {return '.';}
"&" {return '&';}

  /* Keywords */
"int" {return INT;}
"string" {return STRING;}
"any" {return ANY;}
"struct" {return STRUCT;}
"void" {return VOID;}

  /* Int keywords */
"u8" {return U8;}
"u16" {return U16;}
"u32" {return U32;}
"u64" {return U64;}

"s8" {return S8;}
"s16" {return S16;}
"s32" {return S32;}
"s64" {return S64;}

"f32" {return F32;}
"f64" {return F64;}

"return" {return RETURN;}
"defer" {return DEFER;}
"sizeof" {return SIZEOF;}

"if" {return IF;}
"while" {return WHILE;}
"else" {return ELSE;}

  /* Comments */
{WHITESPACE}+ {/*Discard */}
{ALPHA}{ALPHANUMERIC}*  {yylval.string = new std::string(yytext); return ID;}
{NONZERODIGIT}{DIGIT}* {yylval.string = new std::string(yytext); return DEC;}
"0" {yylval.string = new std::string(yytext); return DEC;} // TODO remove after adding OCT

\"(\\.|[^"\\])*\" {yylval.string = new std::string(yytext); return SSTRING;}

  /* //" this line is here because my editor is stupid. */

"/*"    {BEGIN(COMMENT);}
<COMMENT>{
   "/*" {++comment_nesting;} 
   "*"+"/"      {if(comment_nesting) --comment_nesting;
                 else BEGIN(INITIAL);}
    \n  {}
    .   {}
}

"//"    {BEGIN(LINE_COMMENT);}
<LINE_COMMENT>\n        {BEGIN(INITIAL);}
<LINE_COMMENT>. {}


. {printf("I didn't understand %s\n", yytext);}

%%

int yywrap(){
    return 1;
}

