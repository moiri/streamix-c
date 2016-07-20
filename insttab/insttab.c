/**
 * A instance table librabry
 *
 * @file    insttab.c
 * @author  Simon Maurer
 *
 */

#include "insttab.h"
#include "defines.h"
#include <stdio.h>

/******************************************************************************/
void inst_net_del_all( inst_net_t** nets )
{
    inst_net_t *net, *tmp;
    HASH_ITER( hh, *nets, net, tmp ) {
        HASH_DEL( *nets, net );  /* delete; users advances to next */
        inst_rec_del_all( &net->nodes );
        free( net );
    }
}

/******************************************************************************/
inst_net_t* inst_net_get( inst_net_t** nets, int scope )
{
    inst_net_t* res = NULL;
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
inst_net_t* inst_net_put( inst_net_t** nets, int scope )
{
    inst_net_t* item = NULL;
    inst_net_t* new_item = NULL;

    // ADD ITEM TO THE INSTANCE TABLE
    // create new item structure
    new_item = ( inst_net_t* )malloc( sizeof( inst_net_t ) );
    new_item->scope = scope;
    new_item->nodes = NULL;
    // check wheter key already exists
    HASH_FIND_INT( *nets, &scope, item );
    if( item == NULL ) {
        // the key is new
        HASH_ADD_INT( *nets, scope, new_item );
    }
    else return NULL;
#if defined(DEBUG) || defined(DEBUG_INST)
    printf( "inst_net_put: add net in scope %d\n", new_item->scope );
#endif // DEBUG
    return new_item;
}

/******************************************************************************/
void inst_rec_del( inst_rec_t** recs, inst_rec_t* rec )
{
#if defined(DEBUG) || defined(DEBUG_INST)
    printf( "inst_rec_del: delete instance %s(%d)\n", rec->name, rec->id );
#endif // DEBUG
    HASH_DEL( *recs, rec );
    free( rec->name );
    free( rec );
}

/******************************************************************************/
void inst_rec_del_all( inst_rec_t** recs )
{
    inst_rec_t *rec, *tmp;
    HASH_ITER( hh, *recs, rec, tmp ) {
        inst_rec_del( recs, rec );
    }
}

/******************************************************************************/
inst_rec_t* inst_rec_get( inst_rec_t** recs, int id )
{
    inst_rec_t* res = NULL;
    HASH_FIND( hh, *recs, &id, sizeof( int ), res );
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
inst_rec_t* inst_rec_put( inst_rec_t** recs, char* name, int id, int line,
        inst_rec_type_t type, struct symrec_s* rec )
{
    inst_rec_t* item = NULL;
    inst_rec_t* new_item = NULL;

    // ADD ITEM TO THE INSTANCE TABLE
    // create new item structure
    new_item = ( inst_rec_t* )malloc( sizeof( inst_rec_t ) );
    new_item->id = id;
    new_item->line = line;
    new_item->type = type;
    new_item->net = rec;
    new_item->name = ( char* )malloc( strlen( name ) + 1 );
    strcpy( new_item->name, name );
    // check whether key already exists
    HASH_FIND( hh, *recs, &id, sizeof( int ), item );
    if( item == NULL ) {
        // id must be unique in the net
        HASH_ADD( hh, *recs, id, sizeof( int ), new_item );
    }
    else return NULL;
#if defined(DEBUG) || defined(DEBUG_INST)
    printf( "inst_rec_put: add net instance %s with id %d\n", new_item->name,
            new_item->id );
#endif // DEBUG
    return new_item;
}

/******************************************************************************/
void inst_rec_replace_id( inst_rec_t** recs, int id_old, int id_new )
{
    inst_rec_t* item = NULL;
    HASH_FIND( hh, *recs, &id_old, sizeof( int ), item );
    HASH_DELETE( hh, *recs, item );
    item->id = id_new;
    HASH_ADD( hh, *recs, id, sizeof( int ), item );
#if defined(DEBUG) || defined(DEBUG_INST)
    printf( "inst_rec_replace: replace inst id %d with %d\n", id_old, id_new );
#endif // DEBUG
}
