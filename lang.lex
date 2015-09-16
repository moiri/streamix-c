/* 
 * The scanner for the coordination language xyz
 *
 * @file    lang.lex
 * @author  Simon Maurer
 *
 * */

%{
    #include <stdio.h>
    #include "lang.tab.h"  // to get the token types that we return
    #define YY_DECL extern int yylex()
    void yyerror (char const *);
    int num_lines = 1;
    int num_errors = 0;
    extern char* src_file_name;
%}
%%
    /* skip whitespaces and CR */
[ \t]           ;
\n              num_lines++;

    /* keywords */
on              return ON;
up              return UP;
down            return DOWN;
side            return SIDE;
in              return IN;
out             return OUT;
box             return BOX;
wrap            return WRAP;
stateless       return STATELESS;
decoupled       return DECOUPLED;
sync            return SYNC;

    /* identifiers */
[a-zA-Z_$][a-zA-Z_$0-9]* {
                yylval.sval = strdup(yytext);
                return IDENTIFIER;
}

    /* operators */
[.|(){}:,]     return *yytext;

    /* anything else is an error */
.               yyerror("invalid character");
%%

void yyerror(const char *s) {
    num_errors++;
    printf("%s:%d: error: %s\n", src_file_name, num_lines, s);
}
