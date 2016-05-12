#include "cgraph.h"
#include "defines.h"

/******************************************************************************/
void cgraph_connect( igraph_t* g, igraph_vector_ptr_t* v1,
        igraph_vector_ptr_t* v2 )
{
    igraph_vector_t edges;
    int i, j;
    igraph_vector_init( &edges, 0 );
    for( i=0; i<igraph_vector_ptr_size( v1 ); i++ ) {
        for( j=0; j<igraph_vector_ptr_size( v2 ); j++ ) {
            igraph_vector_push_back( &edges,
                    *( int* )igraph_vector_ptr_e( v1, i ) );
            igraph_vector_push_back( &edges,
                    *( int* )igraph_vector_ptr_e( v2, j ) );
        }
    }
    igraph_add_edges( g, &edges, 0 );
    igraph_vector_destroy( &edges );
}

/******************************************************************************/
void cgraph_connect_dir( igraph_t* g, int id1, int id2, int mode1, int mode2 )
{
    int id_from = id1;
    int id_to = id2;
    if( ( mode2 == VAL_OUT ) || ( mode1 == VAL_IN ) ) {
        id_to = id_from;
        id_from = id2;
    }
    igraph_add_edge( g, id_from, id_to );
}

/******************************************************************************/
int cgraph_merge_vertices( igraph_t* g, int id1, int id2 )
{
    igraph_vector_t v_new;
    int idx;
    int id_new = 0;
    int id_low = id1;
    int id_high = id2;
    if( id2 < id1 ) {
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
    igraph_contract_vertices( g, &v_new, NULL );

    // elements starting from this id need to be altered in the insttab
    return id_high + 1;
}
