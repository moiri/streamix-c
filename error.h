/* 
 * Error message helper function
 *
 * @file    error.h
 * @author  Simon Maurer
 *
 * */

#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include "defines.h"

extern char* yytext;
extern int yylineno;
extern int yynerrs;
extern char* __src_file_name;

/* handle errors with the bison error function */
void yyerror ( void**, const char* );

void report_yyerror( const char*, int );

#endif // ERROR_H

