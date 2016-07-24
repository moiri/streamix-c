/**
 * Small library to handle the dependancy graph according to the semantics of
 * streamix
 *
 * @file    smxgraph.c
 * @author  Simon Maurer
 *
 */

#include "smxgraph.h"
#include "defines.h"

/******************************************************************************/
void dgraph_vptr_to_v( igraph_vector_ptr_t* vptr, igraph_vector_t* v )
{
    int i;
    igraph_vector_resize( v, igraph_vector_ptr_size( vptr ) );
    for( i = 0; i < igraph_vector_ptr_size( vptr ); i++ )
        VECTOR( *v )[i] = *( int* )igraph_vector_ptr_e( vptr, i );
}

/******************************************************************************/
void dgraph_connect_1( igraph_t* g, int id1, int id2, int mode1, int mode2,
        const char* name )
{
    int id_from = id1;
    int id_to = id2;
    int id_edge;
    if( ( mode2 == PORT_MODE_OUT ) || ( mode1 == PORT_MODE_IN ) ) {
        id_to = id_from;
        id_from = id2;
    }
    igraph_add_edge( g, id_from, id_to );
    igraph_get_eid( g, &id_edge, id_from, id_to, 0, 0 );
    igraph_cattribute_EAS_set( g, "label", id_edge, name );
}

/******************************************************************************/
int dgraph_merge_vertice_1( igraph_t* g, int id1, int id2 )
{
    igraph_vector_t v_new;
    igraph_attribute_combination_t comb;
    int idx;
    int id_new = 0;
    int id_low = id1;
    int id_high = id2;
    if( id_high < id_low ) {
        id_low = id2;
        id_high = id1;
    }
    igraph_vector_init( &v_new, igraph_vcount( g ) );
    for( idx=0; idx<igraph_vcount( g ); idx++ ) {
        if( idx == id_high ) VECTOR( v_new )[ idx ] = id_low;
        else {
            VECTOR( v_new )[ idx ] = id_new;
            id_new++;
        }
    }
    igraph_attribute_combination( &comb, "label",
            IGRAPH_ATTRIBUTE_COMBINE_FIRST, "func",
            IGRAPH_ATTRIBUTE_COMBINE_FIRST, "inst",
            IGRAPH_ATTRIBUTE_COMBINE_FIRST, IGRAPH_NO_MORE_ATTRIBUTES );
    igraph_contract_vertices( g, &v_new, &comb );
    igraph_vector_destroy( &v_new );

    // id of deleted element
    return id_high;
}
