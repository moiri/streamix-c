#include "insttab.h"
#include "defines.h"
#include <stdio.h>

/******************************************************************************/
symrec* instrec_get( symrec** insttab, char* name, int scope, int id )
{
    symrec* item = NULL;
    char key[ strlen( name ) + 1 + CONST_SCOPE_LEN ];

#if defined(DEBUG) || defined(DEBUG_INST)
    printf( "search instance '%s' with id %d in scope %d\n", name, id, scope );
#endif // DEBUG

    // generate key
    sprintf( key, "%s%d", name, scope );
    HASH_FIND_STR( *insttab, key, item );

    if( id == -1 ) {
        // if no id is available, collision handling must be done manually
#if defined(DEBUG) || defined(DEBUG_INST)
        if( item != NULL )
            printf( "found instances of '%s'(%s) in scope %d\n", item->name,
                    ( ( struct inst_attr*) item->attr )->net->name,
                    item->scope );
#endif // DEBUG
        return item;
    }

    // find the instance with the matching id and handle key collisions
    while( item != NULL ) {
        if( strlen( item->name ) == strlen( name )
            && memcmp( item->name, name, strlen( name ) ) == 0
            && item->scope == scope
            && ( ( struct inst_attr* )item->attr )->id == id ) {
#if defined(DEBUG) || defined(DEBUG_INST)
            printf( "found instance %s with id %d in scope %d\n", item->name,
                    ( ( struct inst_attr* )item->attr)->id, item->scope );
#endif // DEBUG
            break; // found a match
        }
        item = item->next;
    }
    return item;
}

/******************************************************************************/
symrec* instrec_put( symrec** insttab, char* name, int scope, int type, int id,
        symrec* rec )
{
    char key[ strlen( name ) + 1 + CONST_SCOPE_LEN ];
    inst_attr* attr = NULL;
    symrec_list* inst_ports = NULL;
    symrec_list* sym_ports = NULL;
    symrec_list* port_list = NULL;
    symrec* item = NULL;
    symrec* new_item = NULL;
    symrec* previous_item = NULL;

    // PREPARE INSTREC ATTRIBUTES
    attr = ( inst_attr* )malloc( sizeof( inst_attr ) );
    attr->id = id;
    attr->net = rec;
    // copy portlist from symtab to insttab
    if( rec != NULL )
        sym_ports = ( ( struct net_attr* )rec->attr )->ports;
    while( sym_ports != NULL  ) {
        inst_ports = ( struct symrec_list* )malloc( sizeof( symrec_list ) );
        inst_ports->rec = sym_ports->rec;
        inst_ports->next = port_list;
        inst_ports->connect_cnt = 0;
        inst_ports->cp_sync = NULL;
        port_list = inst_ports;
        sym_ports = sym_ports->next;
    }
    attr->ports = port_list;

    // ADD ITEM TO THE INSTANCE TABLE
    // generate key
    sprintf( key, "%s%d", name, scope );
    // create new iten structure
    new_item = ( symrec* )malloc( sizeof( symrec ) );
    new_item->scope = scope;
    new_item->type = type;
    new_item->key = ( char* )malloc( strlen( key ) + 1 );
    strcpy( new_item->key, key );
    new_item->attr = attr;
    new_item->name = ( char* )malloc( strlen( name ) + 1 );
    strcpy( new_item->name, name );
    new_item->attr = attr;
    // check wheter key already exists
    HASH_FIND_STR( *insttab, key, item );
    if( item == NULL ) {
        // the key is new
        HASH_ADD_KEYPTR( hh, *insttab, new_item->key, strlen( key ), new_item );
    }
    else {
        // a collision accured or multiple instances of a net have been spawned
        // -> add the new item to the end of the linked list
        do {
            previous_item = item; // remember the last item of the list
            item = item->next;
        }
        while( item != NULL );

        previous_item->next = new_item;
    }
#if defined(DEBUG) || defined(DEBUG_INST)
    printf( "put net instance %s with id %d in scope %d\n", new_item->name,
            ( ( struct inst_attr* )new_item->attr)->id, new_item->scope );
#endif // DEBUG
    return new_item;
}
