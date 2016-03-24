#include "error.h"

void report_yyerror( const char* msg, int line )
{
    yylineno = line;
    yynerrs++;
    yyerror( msg );
}
