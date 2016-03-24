/* 
 * Global variables and such
 *
 * @file    globals.h
 * @author  Simon Maurer
 *
 * */

#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include "defines.h"

/* handle errors with the bison error function */
extern void yyerror ( const char* );
extern int yylineno;
extern int yynerrs;
char __error_msg[ CONST_ERROR_LEN ];

void yyerror ( const char* );
void report_yyerror( const char*, int );

#endif // ERROR_H

