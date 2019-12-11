WHITESPACE      [ \t\v\n\r\f]

%{
    // C Code goes here
int comment_nesting = 0;
%}

%x COMMENT
%x LINE_COMMENT

%%

{WHITESPACE}+ {/*Discard */}

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

