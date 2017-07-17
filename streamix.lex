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
    extern int yyerror(void*, const char *);
%}
%option noinput
%option nounput
%option yylineno
%option noyywrap

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
up              {yylval.ival = PORT_CLASS_UP;return UP;}
down            {yylval.ival = PORT_CLASS_DOWN;return DOWN;}
side            {yylval.ival = PORT_CLASS_SIDE;return SIDE;}
in              {yylval.ival = PORT_MODE_IN;return IN;}
out             {yylval.ival = PORT_MODE_OUT;return OUT;}
box             {yylval.ival = PARSE_ATTR_BOX;return BOX;}
wrapper         {yylval.ival = PARSE_ATTR_WRAP;return WRAPPER;}
net             {yylval.ival = PARSE_ATTR_NET;return NET;}
connect         return CONNECT;
pure            {yylval.ival = PARSE_ATTR_STATELESS;return STATELESS;}
decoupled       {yylval.ival = PARSE_ATTR_DECOUPLED;return DECOUPLED;}
static          {yylval.ival = PARSE_ATTR_STATIC;return STATIC;}
tt              return TT;
tb              return TB;

    /* identifiers */
[a-zA-Z_$][a-zA-Z_$0-9]* {
                yylval.sval = strdup(yytext);
                return IDENTIFIER;
}
    /* time */
[1-9][0-9]*s {
                char *yycopy = strdup( yytext );
                yycopy[strlen( yycopy ) - 1] = 0;
                yylval.ival = atoi( yycopy );
                return TIME_SEC;
}
[1-9][0-9]{0,2}ms {
                char *yycopy = strdup( yytext );
                yycopy[strlen( yycopy ) - 2] = 0;
                yylval.ival = atoi( yycopy );
                yylval.ival *= 1000000;
                return TIME_NSEC;
}
[1-9][0-9]{0,5}us {
                char *yycopy = strdup( yytext );
                yycopy[strlen( yycopy ) - 2] = 0;
                yylval.ival = atoi( yycopy );
                yylval.ival *= 1000;
                return TIME_NSEC;
}
[1-9][0-9]{0,8}ns {
                char *yycopy = strdup( yytext );
                yycopy[strlen( yycopy ) - 2] = 0;
                yylval.ival = atoi( yycopy );
                return TIME_NSEC;
}

    /* channel length */
[1-9][0-9]* {
                yylval.ival = atoi( yytext );
                return BUFLEN;
}

    /* operators */
[.:|!(){},*=\[\]]     return *yytext;

    /* anything else is an error */
.               yyerror( NULL, "invalid character" );
%%

