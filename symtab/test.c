#include "symtab.h"
#include <string.h>
#include <stdio.h>

int main( ) {
    symrec* symbols = NULL;
    symrec* res = NULL;
    UT_array* scope_stack = NULL; // stack to handle the scope
    int error_cnt = 0;
    char name[] = "test";
    int scope = 4;
    int type = 1;
    int line = 122;

    utarray_new( scope_stack, &ut_int_icd );
    utarray_push_back( scope_stack, &scope );
    symrec_put( &symbols, name, scope, type, NULL, line );
    res = symrec_get( &symbols, scope_stack, name, line );

    if( res == NULL ) {
        printf( "error: record '%s' in scope %d found\n", name, scope );
        error_cnt++;
    }
    else {
        if( res->scope != scope ) {
            printf( "error: expected scope=%d but got scope=%d\n", scope,
                    res->scope );
            error_cnt++;
        }
        if( res->type != type ) {
            printf( "error: expected type=%d but got type=%d\n", type,
                    res->type );
            error_cnt++;
        }
        if( strcmp( res->name, name ) != 0 ) {
            printf( "error: expected name=%s but got name=%s\n", name,
                    res->name );
            error_cnt++;
        }
    }

    if( error_cnt == 0 ) printf( "success!\n" );

    return 0;
}
