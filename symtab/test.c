#include "symtab.h"
#include "defines.h"
#include <string.h>
#include <stdio.h>

int main( ) {
    symrec_t* symbols = NULL;
    symrec_t* rec = NULL;
    symrec_t* res = NULL;
    symrec_list_t* ports = NULL;
    attr_box_t* attr_box = NULL;
    attr_port_t* attr_port = NULL;
    UT_array* scope_stack = NULL; // stack to handle the scope
    int error_cnt = 0;
    char name_box[] = "test_box";
    char name_box_func[] = "func_box";
    char name_net[] = "test_net";
    char name_port1[] = "test_port1";
    char name_port2[] = "test_port2";
    char name_proto[] = "test_proto";
    char name_wrap[] = "test_wrap";
    int scope = 4;
    int type = 1;
    int line = 122;
    bool attr_pure = false;

    utarray_new( scope_stack, &ut_int_icd );
    utarray_push_back( scope_stack, &scope );

    attr_port = symrec_attr_create_port( "int", VAL_IN, VAL_DOWN, false, -1 );
    rec = symrec_create_port( name_port1, scope, line, attr_port );
    ports = malloc( sizeof( symrec_list_t ) );
    ports->rec = rec;
    attr_port = symrec_attr_create_port( "int", VAL_IN, VAL_DOWN, false, -1 );
    rec = symrec_create_port( name_port2, scope, line, attr_port );
    ports->next = malloc( sizeof( symrec_list_t ) );
    ports->next->rec = rec;
    ports->next->next = NULL;
    attr_box = symrec_attr_create_box( attr_pure, name_box_func, ports );
    rec = symrec_create_box( name_box, scope, line, attr_box );
    symrec_put( &symbols, rec );
    res = symrec_get( &symbols, scope_stack, name_box, line );

    if( res == NULL ) {
        printf( "error: record '%s' in scope %d found\n", name_box, scope );
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
        if( strcmp( res->name, name_box ) != 0 ) {
            printf( "error: expected name=%s but got name=%s\n", name_box,
                    res->name );
            error_cnt++;
        }
        if( strcmp( res->attr_box->impl_name, name_box_func ) != 0 ) {
            printf( "error: expected func_name=%s but got name=%s\n",
                    name_box_func, res->attr_box->impl_name );
            error_cnt++;
        }
        if( res->attr_box->attr_pure != false ) {
            printf( "error: expected pure=%d but got pure=%d\n", attr_pure,
                    res->attr_box->attr_pure );
            error_cnt++;
        }
    }

    if( error_cnt == 0 ) printf( "success!\n" );

    return 0;
}
