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
left            {yylval.ival = PORT_CLASS_UP;return UP;}
right           {yylval.ival = PORT_CLASS_DOWN;return DOWN;}
side            {yylval.ival = PORT_CLASS_SIDE;return SIDE;}
in              {yylval.ival = PORT_MODE_IN;return IN;}
out             {yylval.ival = PORT_MODE_OUT;return OUT;}
box             {yylval.ival = PARSE_ATTR_BOX;return BOX;}
wrapper         {yylval.ival = PARSE_ATTR_WRAP;return WRAPPER;}
net             {yylval.ival = PARSE_ATTR_NET;return NET;}
connect         return CONNECT;
pure            {yylval.ival = PARSE_ATTR_STATELESS;return STATELESS;}
decoupled       {yylval.ival = PARSE_ATTR_DECOUPLED;return DECOUPLED;}
coupled         {yylval.ival = PARSE_ATTR_COUPLED;return COUPLED;}
static          {yylval.ival = PARSE_ATTR_STATIC;return STATIC;}
extern          {yylval.ival = PARSE_ATTR_EXTERN;return EXTERN;}
open            {yylval.ival = PARSE_ATTR_OPEN;return OPEN;}
tt              return TT;
tb              return TB;
tf              return TF;

    /* identifiers */
[a-zA-Z_$][a-zA-Z_$0-9]* {
                yylval.sval = strdup(yytext);
                return IDENTIFIER;
}
    /* time */
[1-9][0-9]*s {
                struct timespec time;
                char *yycopy = strdup( yytext );
                yycopy[strlen( yycopy ) - 1] = 0;
                time.tv_sec = atoi( yycopy );
                time.tv_nsec = 0;
                yylval.tval = time;
                free( yycopy );
                return TIME_SEC;
}
[1-9][0-9]*ms {
                struct timespec time;
                char *yycopy = strdup( yytext );
                int len = strlen( yycopy );
                int digits = 3;
                int fact = 1000000;
                yycopy[len - 2] = 0;
                len -= 2;
                if( len > digits ) {
                    time.tv_nsec = atoi( &yycopy[len - digits] ) * fact;
                    yycopy[len - digits] = 0;
                    time.tv_sec = atoi( yycopy );
                }
                else {
                    time.tv_nsec = atoi( yycopy ) * fact;
                    time.tv_sec = 0;
                }
                yylval.tval = time;
                free( yycopy );
                return TIME_MSEC;
}
[1-9][0-9]*us {
                struct timespec time;
                char *yycopy = strdup( yytext );
                int len = strlen( yycopy );
                int digits = 6;
                int fact = 1000;
                yycopy[len - 2] = 0;
                len -= 2;
                if( len > digits ) {
                    time.tv_nsec = atoi( &yycopy[len - digits] ) * fact;
                    yycopy[len - digits] = 0;
                    time.tv_sec = atoi( yycopy );
                }
                else {
                    time.tv_nsec = atoi( yycopy ) * fact;
                    time.tv_sec = 0;
                }
                yylval.tval = time;
                free( yycopy );
                return TIME_USEC;
}
[1-9][0-9]*ns {
                struct timespec time;
                char *yycopy = strdup( yytext );
                int len = strlen( yycopy );
                int digits = 9;
                yycopy[len - 2] = 0;
                len -= 2;
                if( len > digits ) {
                    time.tv_nsec = atoi( &yycopy[len - digits] );
                    yycopy[len - digits] = 0;
                    time.tv_sec = atoi( yycopy );
                }
                else {
                    time.tv_nsec = atoi( yycopy );
                    time.tv_sec = 0;
                }
                yylval.tval = time;
                free( yycopy );
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
