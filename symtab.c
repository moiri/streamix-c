#include <stdlib.h> /* For malloc in symbol table */
#include <string.h> /* For strcmp in symbol table */
#include <stdio.h>
#include "symtab.h"


/******************************************************************************/
symrec* symrec_get( symrec** symtab, char *name ) {
    symrec* item;
    HASH_FIND_STR( *symtab, name, item );
    return item;
}

/******************************************************************************/
void symrec_put( symrec** symtab, char *name, int scope, int type ) {
    symrec* item;
    symrec* new_item;
    HASH_FIND_STR( *symtab, name, item );
    /* new key */
    if( item == NULL ) {
        item = ( symrec* )malloc( sizeof( symrec ) );
        item->scope = scope;
        item->type = type;
        item->name = ( char* )malloc( strlen( name ) + 1 );
        strcpy (item->name, name);
        HASH_ADD_KEYPTR( hh, *symtab, item->name, strlen(item->name), item );
    }
    /* hash exists but eiter key or scope is different */
    else if( (item->scope != scope)
            || ( strlen( item->name ) != strlen( name )
                && memcmp( item->name, name, strlen( name ) ) != 0 ) ) {
        new_item = ( symrec* )malloc( sizeof( symrec ) );
        new_item->scope = scope;
        new_item->type = type;
        new_item->name = ( char* )malloc( strlen( name ) + 1 );
        strcpy (new_item->name, name);
        item->next = new_item;
    }
}
