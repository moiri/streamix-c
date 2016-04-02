/* 
 * The scanner for the coordination language xyz
 *
 * @file    lang.lex
 * @author  Simon Maurer
 *
 * */

%{
    #include <stdio.h>
    #include "streamix.tab.h"  // to get the token types that we return
    #include "defines.h"
    #define YY_DECL extern int yylex()
    extern int yyerror(const char *);
%}
%option noinput
%option nounput
%option yylineno

%x comment
%%
    /* skip whitespaces and CR */
[ \t]           ;
\n              ++yylloc.last_line;

    /* ignore comments */
"/*"         BEGIN(comment);

<comment>[^*\n]*        /* eat anything that's not a '*' */
<comment>"*"+[^*/\n]*   /* eat up '*'s not followed by '/'s */
<comment>\n             ++yylloc.last_line;
<comment>"*"+"/"        BEGIN(INITIAL);

"//".*          { /* DO NOTHING */ }

    /* keywords */
up              {yylval.ival = VAL_UP;return UP;}
down            {yylval.ival = VAL_DOWN;return DOWN;}
side            {yylval.ival = VAL_SIDE;return SIDE;}
in              {yylval.ival = VAL_IN;return IN;}
out             {yylval.ival = VAL_OUT;return OUT;}
box             {yylval.ival = VAL_BOX;return BOX;}
wrapper         {yylval.ival = VAL_WRAPPER;return WRAPPER;}
net             {yylval.ival = VAL_NET;return NET;}
link            return LINK;
connect         return CONNECT;
stateless       {yylval.ival = VAL_STATELESS;return STATELESS;}
decoupled       {yylval.ival = VAL_DECOUPLED;return DECOUPLED;}
static          {yylval.ival = VAL_STATIC;return STATIC;}
sync            return SYNC;

    /* identifiers */
[a-zA-Z_$][a-zA-Z_$0-9]* {
                yylval.sval = strdup(yytext);
                return IDENTIFIER;
}

    /* operators */
[.|(){}:,*=]     return *yytext;

    /* anything else is an error */
.               yyerror("invalid character");
%%

