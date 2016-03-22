/* 
 * Global variables and such
 *
 * @file    globals.h
 * @author  Simon Maurer
 *
 * */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>
#include "defines.h"

/* handle errors with the bison error function */
extern void yyerror ( const char* );
extern int yylineno;
extern int yynerrs;
extern int __node_id;
char __error_msg[ CONST_ERROR_LEN ];

void report_yyerror( const char* msg, int line )
{
    yylineno = line;
    yynerrs++;
    yyerror( msg );
}

#endif // GLOBALS_H

