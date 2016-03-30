#include "insttab.h"
#include "defines.h"
#include <stdio.h>

/******************************************************************************/
inst_net* inst_net_get( inst_net** nets, int scope )
{
    inst_net* res = NULL;
    HASH_FIND_INT( *nets, &scope, res );
#if defined(DEBUG) || defined(DEBUG_INST)
    if( res != NULL )
        printf( "inst_net_get: found nets in scope %d\n", res->scope );
    else
        printf( "inst_net_get: no nets found in scope %d\n", scope );
#endif // DEBUG
    return res;
}

/******************************************************************************/
inst_net* inst_net_put( inst_net** nets, int scope, inst_rec** recs_name,
        inst_rec** recs_id, igraph_t g, net_con* con )
{
    inst_net* item = NULL;
    inst_net* new_item = NULL;
    inst_net* previous_item = NULL;

    // ADD ITEM TO THE INSTANCE TABLE
    // create new item structure
    new_item = ( inst_net* )malloc( sizeof( inst_net ) );
    new_item->scope = scope;
    new_item->recs_id = recs_id;
    new_item->recs_name = recs_name;
    new_item->g = g;
    new_item->con = con;
    // check wheter key already exists
    HASH_FIND_INT( *nets, &scope, item );
    if( item == NULL ) {
        // the key is new
        HASH_ADD_INT( *nets, scope, new_item );
    }
    else {
        // multiple nets in the same scope
        // -> add the new item to the end of the linked list
        do {
            previous_item = item; // remember the last item of the list
            item = item->next;
        }
        while( item != NULL );

        previous_item->next = new_item;
    }
#if defined(DEBUG) || defined(DEBUG_INST)
    printf( "inst_net_put: add net in scope %d\n", new_item->scope );
#endif // DEBUG
    return new_item;
}

/******************************************************************************/
inst_rec* inst_rec_get_id( inst_rec** recs, int id )
{
    inst_rec* res = NULL;
    HASH_FIND( hh1, *recs, &id, sizeof( int ), res );
#if defined(DEBUG) || defined(DEBUG_INST)
    if( res != NULL )
        printf( "inst_rec_get_id: found instance '%s'(%d)\n", res->name,
                res->id );
    else
        printf( "inst_rec_get_id: found no instances with id '%d'\n", id );
#endif // DEBUG
    return res;
}

/******************************************************************************/
inst_rec* inst_rec_get_name( inst_rec** recs, char* name )
{
    inst_rec* res = NULL;
    HASH_FIND( hh2, *recs, name, strlen( name ), res );
#if defined(DEBUG) || defined(DEBUG_INST)
    if( res != NULL )
        printf( "inst_rec_get_name: found instances of '%s'\n", res->name );
    else
        printf( "inst_rec_get_name: found no instances of '%s'\n", name );
#endif // DEBUG
    return res;
}

/******************************************************************************/
inst_rec* inst_rec_put( inst_rec** recs_name, inst_rec** recs_id, char* name,
        int id, symrec* rec )
{
    symrec_list* inst_ports = NULL;
    symrec_list* sym_ports = NULL;
    symrec_list* port_list = NULL;
    inst_rec* item = NULL;
    inst_rec* new_item = NULL;
    inst_rec* previous_item = NULL;

    // ADD ITEM TO THE INSTANCE TABLE
    // create new item structure
    new_item = ( inst_rec* )malloc( sizeof( inst_rec ) );
    new_item->id = id;
    new_item->net = rec;
    new_item->name = ( char* )malloc( strlen( name ) + 1 );
    strcpy( new_item->name, name );
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
    new_item->ports = port_list;
    // check whether key already exists
    HASH_FIND( hh1, *recs_id, &id, sizeof( int ), item );
    if( item == NULL ) {
        // id must be unique in the net
        HASH_ADD( hh1, *recs_id, id, sizeof( int ), new_item );
    }
    item = NULL;
    HASH_FIND( hh2, *recs_name, name, strlen( name ), item );
    if( item == NULL ) {
        // the key is new
        HASH_ADD_KEYPTR( hh2, *recs_name, name, strlen( name ), new_item );
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
    printf( "inst_rec_put: add net instance %s with id %d\n", new_item->name,
            new_item->id );
#endif // DEBUG
    return new_item;
}
