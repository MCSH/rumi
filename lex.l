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
"(" {return '(';}
")" {return ')';}
"{" {return '{';}
"}" {return '}';}
";" {return ';';}
":" {return ':';}
"=" {return '=';}

  /* Keywords */
"int" {return INT;}
"return" {return RETURN;}

  /* Comments */
{WHITESPACE}+ {/*Discard */}
{ALPHA}{ALPHANUMERIC}*  {yylval.string = new std::string(yytext); return ID;}
{NONZERODIGIT}{DIGIT}* {yylval.string = new std::string(yytext); return DEC;}

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

