/* 
 * Sample Scanner1: 
 * Description: Replace the string "username" from standard input 
 *              with the user's login name (e.g. lgao)
 * Usage: (1) $ flex sample1.lex
 *        (2) $ gcc lex.yy.c -lfl
 *        (3) $ ./a.out
 *            stdin> username
 *	      stdin> Ctrl-D
 * Question: What is the purpose of '%{' and '%}'?
 *           What else could be included in this section?
 */

%{
    #include <stdio.h>
    #include "lang.tab.h"  // to get the token types that we return
    #define YY_DECL extern int yylex()
    void yyerror (char const *);
    int num_lines = 0;
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
    printf("%s:%d: %s\n", src_file_name, num_lines, s);
}
