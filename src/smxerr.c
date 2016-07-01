/**
 * Error message helper function
 *
 * @file    smxerr.c
 * @author  Simon Maurer
 *
 */

#include <string.h>
#include "smxerr.h"

void report_yyerror( const char* msg, int line )
{
    yylineno = line;
    yynerrs++;
    yyerror( NULL, msg );
}

/*
 * error function of bison
 *
 * @param: char* s:  error string
 * */
void yyerror( void** ast, const char* s ) {
    if( strlen(yytext) == 0 )
        printf( "%s: %d: %s\n", __src_file_name, yylineno, s );
    else
        printf( "%s: %d: %s '%s'\n", __src_file_name, yylineno, s, yytext );
}
