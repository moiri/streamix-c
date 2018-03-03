/**
 * Error message helper function
 *
 * @file    smxerr.h
 * @author  Simon Maurer
 *
 */

#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include "defines.h"

extern char* yytext;
extern int yylineno;
extern int yynerrs;
extern char* __src_file_name;

#define CONST_ERROR_LEN 256

/* handle errors with the bison error function */
void yyerror ( void**, const char* );

void report_yyerror( const char*, int );

#define ERR_WARNING "warning"
#define ERR_ERROR   "error"

/* ERRORS */
#define ERROR_ALL_IN_DEC\
    "%s: all input ports are decoupled"

#define ERROR_UNDEF_ID\
    "%s: use of undeclared identifier '%s'"

#define ERROR_DUPLICATE_ID\
    "%s: redefinition of '%s'"

#define ERROR_BAD_MODE\
    "%s: conflicting modes of ports '%s' in '%s'(%d) and '%s'(%d) (line %d)"

// currently not used
#define ERROR_BAD_MODE_SIDE\
    "%s: conflicting modes of side port '%s' in '%s'(%d)"

#define ERROR_NO_NET_CON\
    "%s: no port connection in serial combinition '%s(%d).%s(%d)'"

#define ERROR_NO_PORT_CON\
    "%s: port '%s' in '%s'(%d) is not connected"

// currently not used
#define ERROR_UNDEF_PORT\
    "%s: use of undeclared port '%s' in '%s'(%d)"

#define ERROR_UNDEF_NET\
    "%s: undefined reference to net '%s'"

#define ERROR_TYPE_CONFLICT\
    "%s: conflicting types for '%s'"

#define ERROR_SMODE_CP\
    "%s: single mode in routing node '%s'(%d)"

#define ERROR_BAD_SIA_PORT\
    "%s: action '%s' does not match the box signature"

#define ERROR_NONDET\
    "%s: nondeterminism on deterministic operation '%s|%s', use '!' instead"

#define ERROR_NO_PORT_CON_CLASS\
    "%s: unconnected port '%s' in '%s'(%d) of serial combinition '%s.%s'\n"\
    " -> for bypassing, use operator ':' or a wrapper"

/* WARNINGS */
// currently not used
#define WARNING_ALTER_CLASS\
    "%s: port class of port '%s' in '%s'(%d) changed from '%s' to '%s'"

#define WARNING_NO_TF\
    "%s: net has no open ports, operator 'tf' has no effect"

#endif // ERROR_H

