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
inst_net* inst_net_put( inst_net** nets, int scope )
{
    inst_net* item = NULL;
    inst_net* new_item = NULL;
    inst_net* previous_item = NULL;
    net_con* con = NULL;
    igraph_t g;

    // ADD ITEM TO THE INSTANCE TABLE
    // create new item structure
    new_item = ( inst_net* )malloc( sizeof( inst_net ) );
    new_item->scope = scope;
    new_item->recs_id = NULL;
    new_item->recs_name = NULL;
    // create new graph
    igraph_empty( &g, 0, true );
    new_item->g = g;
    // initialize the connection vectors
    con = ( net_con* )malloc( sizeof( net_con ) );
    igraph_vector_init( &con->left, 0 );
    igraph_vector_init( &con->right, 0 );
    new_item->con = con;
    // check wheter key already exists
    HASH_FIND_INT( *nets, &scope, item );
    if( item == NULL ) {
        // the key is new
        HASH_ADD_INT( *nets, scope, new_item );
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
    printf( "inst_net_put: add net in scope %d\n", new_item->scope );
#endif // DEBUG
    return new_item;
}

/******************************************************************************/
inst_rec* inst_rec_get_id( inst_rec** recs, int id )
{
    inst_rec* res = NULL;
    HASH_FIND( hh_id, *recs, &id, sizeof( int ), res );
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
    HASH_FIND( hh_name, *recs, name, strlen( name ), res );
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
        int id, int line, int type, symrec* rec )
{
    inst_rec* item = NULL;
    inst_rec* new_item = NULL;
    inst_rec* previous_item = NULL;

    // ADD ITEM TO THE INSTANCE TABLE
    // create new item structure
    new_item = ( inst_rec* )malloc( sizeof( inst_rec ) );
    new_item->id = id;
    new_item->line = line;
    new_item->type = type;
    new_item->net = rec;
    new_item->name = ( char* )malloc( strlen( name ) + 1 );
    strcpy( new_item->name, name );
    // check whether key already exists
    HASH_FIND( hh_id, *recs_id, &id, sizeof( int ), item );
    if( item == NULL ) {
        // id must be unique in the net
        HASH_ADD( hh_id, *recs_id, id, sizeof( int ), new_item );
    }
    /* else { printf( "ERROR: Something went wrong!" ); } */
    item = NULL;
    HASH_FIND( hh_name, *recs_name, name, strlen( name ), item );
    if( item == NULL ) {
        // the key is new
        HASH_ADD_KEYPTR( hh_name, *recs_name, name, strlen( name ), new_item );
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

/******************************************************************************/
void inst_rec_del( inst_rec** recs_name, inst_rec** recs_id, inst_rec* rec )
{
    inst_rec* rec_name;
    rec_name = inst_rec_get_name( recs_name, rec->name );
    // delete name entry only if no other record with the same name exists
    if( rec_name->next == NULL ) HASH_DELETE( hh_name, *recs_id, rec );
    HASH_DELETE( hh_id, *recs_id, rec );
    free( rec->name );
    free( rec );
}
