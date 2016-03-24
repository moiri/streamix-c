#include "symtab.h"
#include "error.h"
#include <stdio.h>

/******************************************************************************/
symrec* symrec_get( symrec** symtab, UT_array* scope_stack, char *name,
        int line )
{
    int* p = NULL;
    symrec* item = NULL;
    char key[ strlen( name ) + 1 + CONST_SCOPE_LEN ];

    /* check whether their scope matches with a scope on the stack */
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
    if( item == NULL ) {
        sprintf( __error_msg, ERROR_UNDEF_ID, ERR_ERROR, name );
        report_yyerror( __error_msg, line );
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
                && ( ( item->type == VAL_BOX || item->type == VAL_WRAPPER )
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
        sprintf( __error_msg, ERROR_DUPLICATE_ID, ERR_ERROR, name );
        report_yyerror( __error_msg, line );
        item = NULL;
    }
#if defined(DEBUG) || defined(DEBUG_SYMB)
    else {
        printf( "added symbol %s in scope %d\n", item->name, item->scope );
    }
#endif // DEBUG
    return item;
}
