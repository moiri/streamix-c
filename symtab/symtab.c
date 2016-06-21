#include "symtab.h"
#include "ast.h"
#include <stdio.h>
#ifdef TESTING
#include "defines.h"
#else
#include "smxerr.h"
#endif

/******************************************************************************/
void symrec_del( symrec** symtab, symrec* rec )
{
    symrec* rec_temp;
    HASH_FIND_STR( *symtab, rec->key, rec_temp );
#if defined(DEBUG) || defined(DEBUG_SYMB)
    printf( "symrec_del: delete record %s in scope %d\n", rec->name,
            rec->scope );
#endif // DEBUG
    // delete name entry only if no other record with the same name exists
    if( rec_temp->next == NULL ) HASH_DELETE( hh, *symtab, rec );
    free( rec->name );
    free( rec->key );
    switch( rec->type ) {
        case VAL_PORT:
        case VAL_SPORT:
            free( ( ( struct port_attr* )rec->attr )->int_name );
            break;
        case AST_BOX:
            free( ( ( struct box_attr* )rec->attr )->impl_name );
            free( ( ( struct box_attr* )rec->attr )->ports );
            break;
        case AST_WRAP:
            free( ( ( struct wrap_attr* )rec->attr )->ports );
            break;
        default:
            ;
    }
    free( rec->attr );
    free( rec );
}

/******************************************************************************/
symrec* symrec_get( symrec** symtab, UT_array* scope_stack, char *name,
        int line )
{
    symrec* item = NULL;
#ifndef TESTING
    char error_msg[ CONST_ERROR_LEN ];
#endif // TESTING

    /* check whether their scope matches with a scope on the stack */
    item = symrec_search( symtab, scope_stack, name );
    if( item == NULL ) {
#ifdef TESTING
        printf( ERROR_UNDEF_ID, ERR_ERROR, name );
        printf( "\n" );
#else
        sprintf( error_msg, ERROR_UNDEF_ID, ERR_ERROR, name );
        report_yyerror( error_msg, line );
#endif // TESTING
    }
#if defined(DEBUG) || defined(DEBUG_SYMB)
    else {
        printf( "found symbol %s in scope %d\n", item->name, item->scope );
    }
#endif // DEBUG
    return item;
}

/******************************************************************************/
symrec* symrec_put( symrec** symtab, char *name, int scope, int type,
        void* attr, int line )
{
    symrec* item = NULL;
    symrec* new_item = NULL;
    symrec* previous_item = NULL;
    bool is_identical = false;
    char key[ strlen( name ) + 1 + CONST_SCOPE_LEN ];
#ifndef TESTING
    char error_msg[ CONST_ERROR_LEN ];
#endif // TESTING

    // generate key
    sprintf( key, "%s%d", name, scope );
    // create new iten structure
    new_item = ( symrec* )malloc( sizeof( symrec ) );
    new_item->scope = scope;
    new_item->type = type;
    new_item->line = line;
    new_item->key = ( char* )malloc( strlen( key ) + 1 );
    strcpy( new_item->key, key );
    new_item->attr = attr;
    new_item->name = ( char* )malloc( strlen( name ) + 1 );
    strcpy( new_item->name, name );
    new_item->attr = attr;
    // check wheter key already exists
    HASH_FIND_STR( *symtab, key, item );
    // the key is new
    if( item == NULL ) {
        HASH_ADD_KEYPTR( hh, *symtab, new_item->key, strlen( key ), new_item );
        item = new_item;
    }
    /* hash exists */
    else {
        // iterate through all entries with the same hash to handle collisions
        // and catch ports with the same name and scope but a with different
        // collections
        do {
            // check whether there is an identical entry (name, scope, and
            // collections are the same; the mode must always be different)
            if( strlen( item->name ) == strlen( name )
                && memcmp( item->name, name, strlen( name ) ) == 0
                && item->scope == scope
                && ( ( ( item->type == AST_NET )
                        || ( item->type == AST_NET_PROTO )
                        || ( item->type == AST_BOX ) )
                    || ( ( type == VAL_PORT || type == VAL_SPORT )
                        /* && ( ( ( struct port_attr* )attr )->mode */
                        /*     == ( ( struct port_attr* )item->attr )->mode ) */
                        && ( ( ( struct port_attr* )attr )->collection
                            == ( ( struct port_attr* )item->attr )->collection )
                    )
                ) ) {
                // found an identical entry -> error
                is_identical = true;
                break;
            }
            previous_item = item;   // remember the last item of the list
            item = item->next;
        }
        while( item != NULL );

        if( !is_identical ) {
            // the item was differen -> add it to the end of the linked list
            previous_item->next = new_item;
            item = new_item;
        }
    }
    if( is_identical ) {
        // the item already existed in the table -> free the allocated space
        // and throw a yyerror
        free( new_item->name );
        free( new_item->key );
        free( new_item );
#ifdef TESTING
        printf( ERROR_DUPLICATE_ID, ERR_ERROR, name );
        printf( "\n" );
#else
        sprintf( error_msg, ERROR_DUPLICATE_ID, ERR_ERROR, name );
        report_yyerror( error_msg, line );
#endif // TESTING
        item = NULL;
    }
#if defined(DEBUG) || defined(DEBUG_SYMB)
    else {
        printf( "added symbol %s in scope %d\n", item->name, item->scope );
    }
#endif // DEBUG
    return item;
}

/******************************************************************************/
symrec* symrec_search( symrec** symtab, UT_array* scope_stack, char *name )
{
    int* p = NULL;
    symrec* item = NULL;
    char key[ strlen( name ) + 1 + CONST_SCOPE_LEN ];
    while( ( p = ( int* )utarray_prev( scope_stack, p ) ) != NULL ) {
        // generate key
        sprintf( key, "%s%d", name, *p );
        HASH_FIND_STR( *symtab, key, item );
        // iterate through all entries with the same key to handle collisions
        while( item != NULL ) {
            if( strlen( item->name ) == strlen( name )
                && memcmp( item->name, name, strlen( name ) ) == 0
                && item->scope == *p ) {
                break; // found a match
            }
            item = item->next;
        }
        if( item != NULL ) break; // found a match
    }
    return item;
}
