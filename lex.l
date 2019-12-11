WHITESPACE      [ \t\v\n\r\f]

%{
    // C Code goes here
%}

%%

{WHITESPACE}+ {/*Discard */}

. {printf("I didn't understand %s\n", yytext);}

%%

int yywrap(){
    return 1;
}

