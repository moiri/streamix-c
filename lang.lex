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
    #include "defines.h"
    #define YY_DECL extern int yylex()
    extern int yyerror(const char *);
%}
%%
    /* skip whitespaces and CR */
[ \t]           ;
\n              ++yylloc.last_line;

    /* keywords */
on              return ON;
up              {yylval.ival = VAL_UP;return UP;}
down            {yylval.ival = VAL_DOWN;return DOWN;}
side            {yylval.ival = VAL_SIDE;return SIDE;}
in              {yylval.ival = VAL_IN;return IN;}
out             {yylval.ival = VAL_OUT;return OUT;}
box             {yylval.ival = VAL_BOX;return BOX;}
net             {yylval.ival = VAL_NET;return NET;}
signal          return SIGNAL;
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

