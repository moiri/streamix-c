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
    extern int yyerror(const char *);
%}
%%
    /* skip whitespaces and CR */
[ \t]           ;
\n              ++yylloc.last_line;

    /* keywords */
on              return ON;
up              {yylval.ival = 0;return UP;}
down            {yylval.ival = 1;return DOWN;}
side            {yylval.ival = 2;return SIDE;}
in              {yylval.ival = 0;return IN;}
out             {yylval.ival = 1;return OUT;}
box             {yylval.ival = 0;return BOX;}
wrap            {yylval.ival = 1;return WRAP;}
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

