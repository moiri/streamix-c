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
instrec_t* instrec_create( char* name, int id, int line, instrec_type_t type )
{
    instrec_t* new_item = NULL;

    // create new item structure
    new_item = ( instrec_t* )malloc( sizeof( instrec_t ) );
    new_item->id = id;
    new_item->line = line;
    new_item->type = type;
    new_item->name = ( char* )malloc( strlen( name ) + 1 );
    strcpy( new_item->name, name );
#if defined(DEBUG) || defined(DEBUG_INST)
    printf( "instrec_create: create instance %s(%d)\n", new_item->name,
            new_item->id );
#endif // DEBUG
    return new_item;
}

/******************************************************************************/
void instrec_destroy( instrec_t* rec )
{
#if defined(DEBUG) || defined(DEBUG_INST)
    printf( "instrec_destroy: delete instance %s(%d)\n", rec->name, rec->id );
#endif // DEBUG
    free( rec->name );
    free( rec );
}

/******************************************************************************/
void instrec_replace_id( instrec_t* rec, int id_old, int id_new )
{
    if( rec->id == id_old ) rec->id = id_new;
#if defined(DEBUG) || defined(DEBUG_INST)
    printf( "instrec_replace_id: replace inst id %d with %d\n", id_old,
            id_new );
#endif // DEBUG
}
